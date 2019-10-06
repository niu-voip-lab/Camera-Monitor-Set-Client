#include "recordLib.h"
#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#include <iostream>

#include <fstream>

#include<sys/time.h>

#include <vector>

int getTimeX()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    double time_in_mill =
        (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
    return time_in_mill;
}
	      
int record(int argc, char *argv[])
{
  int i;
  int err;
  char *buffer;
  int buffer_frames = 128;
  unsigned int rate = 44100;
  snd_pcm_t *capture_handle;
  snd_pcm_hw_params_t *hw_params;
	snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;

  if ((err = snd_pcm_open (&capture_handle, argv[1], SND_PCM_STREAM_CAPTURE, 0)) < 0) {
    fprintf (stderr, "cannot open audio device %s (%s)\n", 
             argv[1],
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "audio interface opened\n");
		   
  if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
    fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "hw_params allocated\n");
				 
  if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
    fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "hw_params initialized\n");
	
  if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    fprintf (stderr, "cannot set access type (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "hw_params access setted\n");
	
  if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, format)) < 0) {
    fprintf (stderr, "cannot set sample format (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "hw_params format setted\n");
	
  if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0) {
    fprintf (stderr, "cannot set sample rate (%s)\n",
             snd_strerror (err));
    exit (1);
  }
	
  fprintf(stdout, "hw_params rate setted\n");

  if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, 2)) < 0) {
    fprintf (stderr, "cannot set channel count (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "hw_params channels setted\n");
	
  if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
    fprintf (stderr, "cannot set parameters (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "hw_params setted\n");
	
  snd_pcm_hw_params_free (hw_params);

  fprintf(stdout, "hw_params freed\n");
	
  if ((err = snd_pcm_prepare (capture_handle)) < 0) {
    fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  fprintf(stdout, "audio interface prepared\n");

  buffer = (char*)malloc(128 * snd_pcm_format_width(format) / 8 * 2);

  fprintf(stdout, "buffer allocated\n");
    
    int tt = getTimeX();

    i=0;

    std::vector<char> raw;

    int ctn = 0;

  while(getTimeX() - tt < 5000)
  {
    if ((err = snd_pcm_readi (capture_handle, buffer, buffer_frames)) != buffer_frames) {
      fprintf (stderr, "read from audio interface failed (%s)\n",
               err, snd_strerror (err));
      exit (1);
    }
    fprintf(stdout, "read %d done\n", ++i);
    raw.insert (raw.begin(), buffer, buffer+buffer_frames);
    ctn += buffer_frames;
  }

    std::cout << "writing" << std::endl;
    std::ofstream audio;
    audio.open("out.pcm");
    audio.write((char *) &raw[0], ctn);
    audio.close();

  free(buffer);

  fprintf(stdout, "buffer freed\n");
	
  snd_pcm_close (capture_handle);
  fprintf(stdout, "audio interface closed\n");

  exit (0);
}