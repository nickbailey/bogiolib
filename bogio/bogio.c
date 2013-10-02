#include <stdlib.h>
#include <string.h>
#include <alloca.h>
#include <unistd.h>
#include <comedi.h>
#include <comedilib.h>
#include <bogio.h>

int bogio_test(void) { return 1; }

bogio_spec *bogio_open(bogio_spec *spec)
{
    unsigned int *chanlist;       /* For Comedi channel flags */
    unsigned int osr;             /* Oversampling rate */
    int i;
    
    /* Maybe the caller didn't have a bogio_spec */
    if (spec == NULL) {
        spec            = calloc(1U, sizeof(bogio_spec));
        /* We have to set these here because 0 is a valid argument */
        spec->subdevice = 0U;
        spec->range     = 0U; /* +/- 4V */
        spec->aref      = AREF_GROUND;
    }
    
    /* Set defaults if the caller didn't supply parameters */
    if (spec->comedi_device == NULL)
        spec->comedi_device = "/dev/comedi0";
    if (spec->sample_rate == 0)
        spec->sample_rate = 1000; /* 1kHz default sample rate */
    if (spec->channels == 0)
        spec->channels = 8;       /* 8 input channels by default */
    /* implied: oversampling = 0 (no oversampling) */
    
    /* Reserve memory for the Comedi command */
    spec->m_cmd = calloc(1, sizeof(comedi_cmd));
    
    /* Try to open the device */
    spec->m_dev = comedi_open(spec->comedi_device);
    if (spec->m_dev == NULL){
        comedi_perror(spec->comedi_device);
        return NULL;
    }
    
    /* Calculate the oversampled rate for the target sample rate */
    spec->oversampling = 0; /* To do! */
    osr = spec->oversampling ? spec->oversampling : 1;
    osr *= (1000000000U/spec->sample_rate);   /* That's 1e9 */
    
    /* Set up channel list */
    chanlist = (unsigned int *)calloc(spec->channels, sizeof(unsigned int));
    for(i=0 ; i < spec->channels ; i++)
        chanlist[i]=CR_PACK(i, spec->range, spec->aref);
    
    i = comedi_get_cmd_generic_timed(spec->m_dev,
                                     spec->subdevice,
                                     spec->m_cmd,
                                     spec->channels,
                                     osr
                                    );
    if (i < 0) {
        fprintf(stderr, "comedi_get_cmd_generic_timed failed\n");
        return NULL;
    }
    
    /* Modify parts of the command */
    spec->m_cmd->chanlist     = chanlist;
    spec->m_cmd->chanlist_len = spec->channels;
    spec->m_cmd->scan_end_arg = spec->channels;
    spec->m_cmd->stop_src     = TRIG_NONE;
    spec->m_cmd->stop_arg     = 0;

    /* comedi_command_test() tests a command to see if the
       trigger sources and arguments are valid for the subdevice.
       If a trigger source is invalid, it will be logically ANDed
       with valid values (trigger sources are actually bitmasks),
       which may or may not result in a valid trigger source.
       If an argument is invalid, it will be adjusted to the
       nearest valid value.  In this way, for many commands, you
       can test it multiple times until it passes.  Typically,
       if you can't get a valid command in two tests, the original
       command wasn't specified very well. */
    i = comedi_command_test(spec->m_dev, spec->m_cmd);
    if (i < 0) {
        comedi_perror("comedi_command_test -- first attempt");
        exit(-1);
    }
    /* fprintf(stderr,"first test returned %d\n",i); */

    i = comedi_command_test(spec->m_dev, spec->m_cmd);
    if (i < 0){
        comedi_perror("comedi_command_test -- second attempt");
        return NULL;
    }
    /* fprintf(stderr,"second test returned %d\n",i); */
    
    if (i != 0){
        fprintf(stderr,"Error preparing command\n");
        return NULL;
    }
    
    /* Make sure the sample rate wasn't modified */
    if (spec->m_cmd->convert_src==TRIG_TIMER && spec->m_cmd->convert_arg)
        spec->sample_rate = 1E9/spec->m_cmd->convert_arg;

    if (spec->m_cmd->scan_begin_src==TRIG_TIMER && spec->m_cmd->scan_begin_arg)
        spec->sample_rate = 1E9/spec->m_cmd->scan_begin_arg;

    /* start the command */
    i = comedi_command(spec->m_dev, spec->m_cmd);
    if (i < 0) {
        comedi_perror("comedi_command");
        return NULL;
    }
    
    /* That worked: report the current settings */
    /* Read the maxdata value and range for each channel;
       it will be used later for normalisation */
    spec->m_max_sample = calloc(spec->channels, sizeof(lsampl_t));
    spec->fsd = calloc(spec->channels, sizeof(comedi_range *));
    for (i = 0 ; i < spec->channels ; i++) {
        spec->m_max_sample[i] = comedi_get_maxdata(spec->m_dev,
                                                   spec->subdevice,
                                                   i);
        spec->fsd[i]          = comedi_get_range(spec->m_dev,
                                                 spec->subdevice,
                                                 i,
                                                 spec->range);
    }

    return spec;
}

bogio_buf *bogio_allocate_buf(const bogio_spec *spec, unsigned int frames)
{
    bogio_sample_t *data;
    bogio_buf *new_buf = (bogio_buf *)malloc(sizeof(bogio_buf));
    
    /* Bail out if allocation of the descriptor structure failed */
    if (new_buf == NULL)
        return NULL;
    
    /* Now try to allocate the data storage */
    if ((data =
         (bogio_sample_t *)calloc(frames, spec->channels * sizeof(bogio_sample_t)))
         == NULL) {
        /* couldn't allocate data buffer; free descriptor and give up */
        free(new_buf);
        return NULL;
    }
    
    /* Populate the new structure and return it */
    new_buf->spf     = spec->channels;
    new_buf->frames  = frames;
    new_buf->samples = data;
    return new_buf;
}

void bogio_release_buf(bogio_buf *buf)
{
    free(buf->samples);
    free(buf);
}

int bogio_read_frames(bogio_buf *buf, unsigned int frames,
                               int blocking, const bogio_spec *spec)
{
    int bytes, i;
    sampl_t *raw; /* Place to store raw samples before normalisation */
    unsigned int framesize = buf->spf * sizeof(sampl_t);
    
    /* Don't read past the end of the buffer */
    if (frames > buf->frames)
        frames = buf->frames;
    
    if (!blocking) {
        /* If we're not going to block, find out how many samples can be read */
        unsigned int frames_ready =
                comedi_get_buffer_contents(spec->m_dev, spec->subdevice) /
                framesize;
        if (frames > frames_ready)
            frames = frames_ready;
    }
    
    /* Reserve a temporary buffer of the I/O device's type */
    raw = (sampl_t *)alloca(frames*framesize);
    comedi_poll(spec->m_dev, spec->subdevice);
//printf("Going to read %u bytes from fd %d, %d available... ", frames*framesize, comedi_fileno(spec->m_dev), comedi_get_buffer_contents(spec->m_dev, spec->subdevice)); fflush(stdout);
    /* Fill it with data */
    bytes = read(comedi_fileno(spec->m_dev), raw, frames*framesize);
//perror("read");
//printf("Got %d\n", bytes);
    /* Ensure the read completed */
    if (bytes <= 0)
        return bytes;
    
    frames = bytes/framesize;
    
    /* Convert to bogio's native type and normalise. */
    for (i = 0; i < frames * buf->spf ; i++)
        buf->samples[i] = (bogio_sample_t)comedi_to_phys(raw[i],
                                                         spec->fsd[i],
                                                         spec->m_max_sample[i]);
    return frames;
}

int bogio_close(bogio_spec *spec)
{
    int r;
    r = comedi_cancel(spec->m_dev, spec->subdevice);
    r |= comedi_close(spec->m_dev);
    free(spec->fsd);
    free(spec->m_max_sample);
    free(spec->m_cmd->chanlist);
    free(spec->m_cmd);
    return r;
}
