#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

int main (int argc, char *argv[])
{
    int err;

    if (argc < 3){
        printf("usage: play DEVICE BUFFER_SIZE\n");
        return 0;
    }

    int BUFFER_SIZE = atoi(argv[2]);

    short buf[BUFFER_SIZE*4];
    snd_pcm_t *playback_handle;
    snd_pcm_hw_params_t *hw_params;

    if ((err = snd_pcm_open (&playback_handle, argv[1], SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        fprintf (stderr, "cannot open audio device %s (%s)\n",
				 argv[1],
				 snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
        fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
				 snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_any (playback_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
				 snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_set_access (playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf (stderr, "cannot set access type (%s)\n",
				 snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_set_format (playback_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
        fprintf (stderr, "cannot set sample format (%s)\n",
				 snd_strerror (err));
        exit (1);
    }

    unsigned int rate = 44100;
    if ((err = snd_pcm_hw_params_set_rate_near (playback_handle, hw_params, &rate, 0)) < 0) {
        fprintf (stderr, "cannot set sample rate (%s)\n",
				 snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_set_channels (playback_handle, hw_params, 2)) < 0) {
        fprintf (stderr, "cannot set channel count (%s)\n",
				 snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params (playback_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot set parameters (%s)\n",
				 snd_strerror (err));
        exit (1);
    }

    snd_pcm_hw_params_free (hw_params);

    if ((err = snd_pcm_prepare (playback_handle)) < 0) {
        fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
				 snd_strerror (err));
        exit (1);
    }

    while (1) {
		if (read(0, buf, BUFFER_SIZE*4) == 0) {
			printf("Early end of file.\n");
            break;
		}

        if ((err = snd_pcm_writei (playback_handle, buf, BUFFER_SIZE)) == -EPIPE) {
			printf("XRUN.\n");
            if ((err = snd_pcm_prepare (playback_handle)) < 0) {
                fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
                         snd_strerror (err));
                break;
            }
		} else if (err != BUFFER_SIZE) {

            fprintf (stderr, "write to audio interface failed (%s)\n",
					 snd_strerror (err));
            break;
        }
    }

    snd_pcm_close (playback_handle);
    return 0;
}


// gcc -lasound play.c -o play
// ./play sysdefault:CARD=PCH 128
