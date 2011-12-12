#ifndef __BOGIO_H__
#define __BOGIO_H__

#include <comedi.h>
#include <comedilib.h>

/*!
 *  \mainpage
 * Hello!
 */

/*!
 *  \defgroup bogio
 *  The bogio (bogen i/o) library wraps low-level comedi i/o
 *  calls to produce an easy-to-use interface for gathering
 *  gestural information from a hardware interface.
 *  @{
 */

int bogio_test(void);

/*!
 * \brief A structure describing a gesture input channel
 */
typedef struct {
    char           *comedi_device; /**< Hardware device name */
    unsigned short  channels;      /**< Number of channels to read */
    unsigned int    sample_rate;   /**< Effective frame rate of converter in Hz */
    unsigned int    oversampling;  /**< Number of readings per frame produced */
    unsigned int    subdevice;     /**< Comedi subdevice from which to read */
    unsigned int    range;         /**< Measurement range */
    unsigned int    aref;          /**< Analogue voltage reference */
    /* Private attributes */
    comedi_cmd     *m_cmd;         /**< \private Comedi command to execute */
    comedi_t       *m_dev;         /**< \private Comedilib device handle*/
    lsampl_t        m_max_sample;  /**< \private Maximum sample value */
} bogio_spec;

/*!
 * \brief Native type used to represent sample values
 */
typedef double bogio_sample_t;

/*!
 * \brief A frame of data, consisting of a given number of channels
 *        presumed to have been read simultaneously
 */
typedef struct {
    unsigned int spf;               /**< Number of samples in each frame */
    unsigned int frames;            /**< Number of frames in this buffer */
    bogio_sample_t *samples;        /**< Beginning of stored frames **/
} bogio_buf;

/*!
 *  \brief Attempts to initialise a gesture input channel.
 *         In the event that a channel specification hint
 *         cannot be satisfied exactly, the appropriate fields
 *         of the supplied specification will be altered to
 *         indicate the capabilities of the channel actually supplied.
 *  \param[in,out] spec a hint containing the desired channel specification.
 *         passing NULL causes a default specification to be allocated.
 *         It is the callers responsibility to free this memory when it
 *         is no longer required.
 *  \return Pointer to (possibly modified or new) bogio_spec on success;
 *         NULL on failure.
 */
bogio_spec *bogio_open(bogio_spec *spec);

/*!
 * \brief  Reads a given number of frames into the supplied buffer.
           Data is normalised in the range 0..1.0.
 * \param[in] buf Buffer into which frames should be written. Regardless
 *         of the number of frames requested, the number written shall
 *         not exceed the frames field of the supplied buffer.
 * \param[in] frames Number of frames requested.
 * \param[in] blocking If non-zero, block until frames have been read.
 *         Otherwise, return immediately with whatever data is available.
 * \param[in] spec Spec of the interface from which to read.
 * \return Number of frames actually read, -1 for error.
 * \todo   Implement oversampling.
 * \bug    Assumes device sends samples of type sample_t. Always true?
 *         No. The manual comedi manual says "Applications should check
 *         the subdevice flag SDF_LSAMPL to determine if the subdevice
 *         uses sampl_t or lsampl_t".
 */
int bogio_read_frames(bogio_buf *buf, unsigned int frames,
                      int blocking, const bogio_spec *spec);

/*!
 * \brief Allocate memory for a bogio_buf suitable for use with the given
 *        bogio_spec. The spf and frames fields are populated ready for use.
 *        The data part of the buffer is initially set to zero.
 * \param [in] spec Specification of the input source.
 * \param [in] frames Buffer size in frames.
 * \return Pointer to newly allocated buffer, NULL on failure.
 */
bogio_buf *bogio_allocate_buf(const bogio_spec *spec, unsigned int frames);

/*!
 * \brief Release memory allocated by calling bogio_allocate_buf()
 * \param [in] buf Buffer to be released.
 */
void bogio_release_buf(bogio_buf *buf);

/*!
 * \brief Close the speficied device.
 * \return 0 on success, -1 on failure.
 */
int bogio_close(bogio_spec *spec);

/*! @} */

#endif
