/***
 * simple.c
 * 
 * About as easy a bogio app as you can get.
 * 
 * Compile to a.out with:
 * 	gcc simple.c `pkg-config --cflags --libs bogio`
 ***/

#include <stdio.h>
#include <bogio.h>

/* Number of frames to read before termination */
#define MAXFRAMES 120

int main(int argc, char *argv[])
{
	bogio_spec bspec;
	bogio_buf  *bbuf;

	int fr, ch;

	bspec.comedi_device = "/dev/comedi0";
	bspec.channels = 8;
	bspec.sample_rate = 2;
	bspec.oversampling = 0;
	bspec.subdevice = 0;
	bspec.range = 0; /* +/- 4.096V */
	bspec.aref = AREF_GROUND;

	/* Open the device to read data */
	bogio_open(&bspec);
	/* Allocate a buffer for 1 frame of data */
	bbuf = bogio_allocate_buf(&bspec, 1U);
	
	/* Carry on reading samples and printing them */
	for (fr = 0; fr < MAXFRAMES; fr++) {
		printf("%02i:", fr);
		/* Read one frame of samples, blocking, into bbuf */
		bogio_read_frames(&bspec, bbuf, 1U, 1);
		for (ch = 0; ch < bbuf->spf; ch++)
			/* Print only these channels... */
			if (ch == 5)
				printf(" %5.3f", bbuf->samples[ch]);
		printf("\n");
	}

	/* Shut sown the comedi framework and free the buffer */
	bogio_close(&bspec);
	bogio_release_buf(bbuf);
	/* That's all folks */
	return 0;
}

