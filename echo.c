#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <alsa/asoundlib.h>


snd_pcm_t * get_pcm (char *pcm_device_name, snd_pcm_stream_t pcm_type)
{
    int err;
    snd_pcm_t *pcm_handle = NULL;
    snd_pcm_hw_params_t *hw_params;

    if ((err = snd_pcm_open (&pcm_handle, pcm_device_name, pcm_type, 0)) < 0) {
        fprintf (stderr, "cannot open audio device %s (%s)\n",
				 pcm_device_name,
				 snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
        fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
				 snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_any (pcm_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
				 snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_set_access (pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf (stderr, "cannot set access type (%s)\n",
				 snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_set_format (pcm_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
        fprintf (stderr, "cannot set sample format (%s)\n",
				 snd_strerror (err));
        exit (1);
    }

    unsigned int rate = 44100;
    if ((err = snd_pcm_hw_params_set_rate_near (pcm_handle, hw_params, &rate, 0)) < 0) {
        fprintf (stderr, "cannot set sample rate (%s)\n",
				 snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_set_channels (pcm_handle, hw_params, 2)) < 0) {
        fprintf (stderr, "cannot set channel count (%s)\n",
				 snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params (pcm_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot set parameters (%s)\n",
				 snd_strerror (err));
        exit (1);
    }

    snd_pcm_hw_params_free (hw_params);

    return pcm_handle;
}


int main (int argc, char *argv[])
{
    int err;
    if (argc < 4){
        printf("usage: play PLAYBACK_DEVICE RECORDER_DEVICE BUFFER_SIZE\n");
        return 0;
    }

    int BUFFER_SIZE = atoi(argv[3]);
    short buf[BUFFER_SIZE*4];

    snd_pcm_t *playback_handle = get_pcm(argv[1], SND_PCM_STREAM_PLAYBACK);
    snd_pcm_t *capture_handle = get_pcm(argv[2], SND_PCM_STREAM_CAPTURE);

    snd_pcm_sw_params_t *sw_params;
    snd_pcm_sframes_t frames_to_deliver;

    /* tell ALSA to wake us up whenever 4096 or more frames
       of playback data can be delivered. Also, tell
       ALSA that we'll start the device ourselves.
    */

    if ((err = snd_pcm_sw_params_malloc (&sw_params)) < 0) {
        fprintf (stderr, "cannot allocate software parameters structure (%s)\n",
				 snd_strerror (err));
        exit (1);
    }
    if ((err = snd_pcm_sw_params_current (playback_handle, sw_params)) < 0) {
        fprintf (stderr, "cannot initialize software parameters structure (%s)\n",
				 snd_strerror (err));
        exit (1);
    }
    if ((err = snd_pcm_sw_params_set_avail_min (playback_handle, sw_params, BUFFER_SIZE)) < 0) {
        fprintf (stderr, "cannot set minimum available count (%s)\n",
				 snd_strerror (err));
        exit (1);
    }
    if ((err = snd_pcm_sw_params_set_start_threshold (playback_handle, sw_params, 0U)) < 0) {
        fprintf (stderr, "cannot set start mode (%s)\n",
				 snd_strerror (err));
        exit (1);
    }
    if ((err = snd_pcm_sw_params (playback_handle, sw_params)) < 0) {
        fprintf (stderr, "cannot set software parameters (%s)\n",
				 snd_strerror (err));
        exit (1);
    }


    if ((err = snd_pcm_prepare (playback_handle)) < 0) {
        fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
				 snd_strerror (err));
        exit (1);
    }
    if ((err = snd_pcm_prepare (capture_handle)) < 0) {
        fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
				 snd_strerror (err));
        exit (1);
    }


    while (1) {

        if ((err = snd_pcm_wait (playback_handle, 1000)) < 0) {
            fprintf (stderr, "poll failed (%s)\n", strerror (err));
            break;
        }

        /* find out how much space is available for playback data */

        if ((frames_to_deliver = snd_pcm_avail_update (playback_handle)) < 0) {
            if (frames_to_deliver == -EPIPE) {
                fprintf (stderr, "an xrun occured\n");
                break;
            } else {
                fprintf (stderr, "unknown ALSA avail update return value (%d)\n",
						 frames_to_deliver);
                break;
            }
        }

        frames_to_deliver = frames_to_deliver > BUFFER_SIZE ? BUFFER_SIZE : frames_to_deliver;

        if ((err = snd_pcm_writei (playback_handle, buf, frames_to_deliver))  == -EPIPE) {
			printf("XRUN.\n");
            if ((err = snd_pcm_prepare (playback_handle)) < 0) {
                fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
                         snd_strerror (err));
                break;
            }

		} else if (err != frames_to_deliver) {
            fprintf (stderr, "write to audio interface failed (%s)\n",
					 snd_strerror (err));
            break;
        }

        // for capture_handle
        /* if ((err = snd_pcm_readi (capture_handle, buf, BUFFER_SIZE)) != BUFFER_SIZE) { */
        /*     printf ("read from audio interface failed (%s)\n", */
		/* 			 snd_strerror (err)); */
        /*     break; */
        /* } */

        /* printf("\nround: %d\n", RAND_MAX); */
        for (int j = 0; j < BUFFER_SIZE*4; ++j) {
            buf[j] = (int)(sin(j*3.1415926536*2/(BUFFER_SIZE*4)) * atoi(argv[4]));

            /* buf[j] = rand() % atoi(argv[4]); */
            /* printf("%d,", buf[j]); */
        }
    }

    snd_pcm_close (capture_handle);
    snd_pcm_close (playback_handle);
    return 0;
}

// gcc -lasound echo.c -o echo
// ./echo sysdefault:CARD=PCH sysdefault:CARD=Microphone
