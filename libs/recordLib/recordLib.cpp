/*
Source : https://cboard.cprogramming.com/linux-programming/167738-sound-recording-using-alsa-lib-pls-help.html
*/

#include "recordLib.h"
#include <fcntl.h>
#include <stdio.h>
#include <alsa/asoundlib.h>

AlsaRecord::AlsaRecord(char* hw)
{
    this->init(hw, SND_PCM_FORMAT_S16_LE, 44100, 2);
}

AlsaRecord::AlsaRecord(char* hw, snd_pcm_format_t format, int rate, int channel)
{
    this->init(hw, format, rate, channel);
}

void AlsaRecord::init(char* hw, snd_pcm_format_t format, int rate, int channel)
{
    int rc, err;
    snd_pcm_hw_params_t* params;

    this->rate = rate;
    this->channel = channel;

    /* Open PCM device for recording (capture). */
    rc = snd_pcm_open(&handle, hw,
        SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        fprintf(stderr,
            "unable to open pcm device: %s\n",
            snd_strerror(rc));
        exit(1);
    }

    if ((err = snd_pcm_hw_params_malloc(&params)) < 0) {
        fprintf(stderr, "cannot allocate hardware parameter structure (%s)\n",
            snd_strerror(err));
        exit(1);
    }

    fprintf(stdout, "hw_params allocated\n");

    if ((err = snd_pcm_hw_params_any(handle, params)) < 0) {
        fprintf(stderr, "cannot initialize hardware parameter structure (%s)\n",
            snd_strerror(err));
        exit(1);
    }

    fprintf(stdout, "hw_params initialized\n");

    if ((err = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf(stderr, "cannot set access type (%s)\n",
            snd_strerror(err));
        exit(1);
    }

    fprintf(stdout, "hw_params access setted\n");

    if ((err = snd_pcm_hw_params_set_format(handle, params, format)) < 0) {
        fprintf(stderr, "cannot set sample format (%s)\n",
            snd_strerror(err));
        exit(1);
    }

    fprintf(stdout, "hw_params format setted\n");
    if ((err = snd_pcm_hw_params_set_rate_near(handle, params, &this->rate, 0)) < 0) {
        fprintf(stderr, "cannot set sample rate (%s)\n",
            snd_strerror(err));
        exit(1);
    }

    fprintf(stdout, "hw_params rate setted\n");

    if ((err = snd_pcm_hw_params_set_channels(handle, params, this->channel)) < 0) {
        fprintf(stderr, "cannot set channel count (%s)\n",
            snd_strerror(err));
        exit(1);
    }

    fprintf(stdout, "hw_params channels setted\n");

    if ((err = snd_pcm_hw_params(handle, params)) < 0) {
        fprintf(stderr, "cannot set parameters (%s)\n",
            snd_strerror(err));
        exit(1);
    }

    fprintf(stdout, "hw_params setted\n");

    snd_pcm_hw_params_free(params);

    fprintf(stdout, "hw_params freed\n");

    if ((err = snd_pcm_prepare(handle)) < 0) {
        fprintf(stderr, "cannot prepare audio interface for use (%s)\n",
            snd_strerror(err));
        exit(1);
    }

    fprintf(stdout, "audio interface prepared\n");

    buffer = (char*)malloc(128 * snd_pcm_format_width(format) / 8 * 2);

    fprintf(stdout, "buffer allocated\n");

    /* Use a buffer large enough to hold one period */
    snd_pcm_hw_params_get_period_size(params, &frames, &dir);
    size = frames * 4; /* 2 bytes/sample, 2 channels */
    buffer = (char*)malloc(size);

    snd_pcm_hw_params_get_period_time(params, &val, &dir);

    
}

int AlsaRecord::getBufferSize()
{
    return this->size;
}

int AlsaRecord::getRate()
{
    return this->rate;
}

int AlsaRecord::getPeriod()
{
    return this->val;
}

int AlsaRecord::record(char* buffer, int length)
{
    int rc = snd_pcm_readi(this->handle, this->buffer, this->frames);
    if (rc == -EPIPE)
    {
        /* EPIPE means overrun */
        fprintf(stderr, "overrun occurred\n");
        snd_pcm_prepare(handle);
    }
    else if (rc < 0)
    {
        fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
    }
    else if (rc != (int)frames)
    {
        fprintf(stderr, "short read, read %d frames\n", rc);
    }

    if(length < this->size)
    {
        memcpy(this->buffer, buffer, length);
        fprintf(stderr, "buffer not enough. %d stored, %d discard\n", length, this->size-length);
    }
    else
    {
        memcpy(buffer, this->buffer, length);
    }
    return this->size;
}

int AlsaRecord::recordDirect(char* buffer, int length)
{
    int rc = snd_pcm_readi(this->handle, buffer, this->frames);
    if (rc == -EPIPE)
    {
        /* EPIPE means overrun */
        fprintf(stderr, "overrun occurred\n");
        snd_pcm_prepare(handle);
    }
    else if (rc < 0)
    {
        fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
    }
    else if (rc != (int)frames)
    {
        fprintf(stderr, "short read, read %d frames\n", rc);
    }

    return this->size;
}

int AlsaRecord::close()
{
    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    free(buffer);
}