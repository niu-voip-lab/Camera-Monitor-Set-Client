#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

class AlsaRecord
{
public:
    AlsaRecord(char* hw);
    AlsaRecord(char* hw, snd_pcm_format_t format, int rate, int channel);
    int getBufferSize();
    int getRate();
    int getPeriod();
    int record(char* buffer, int length);
    int recordDirect(char* buffer, int length);
    int close();
private:
    snd_pcm_t* handle;
    char* buffer;
    snd_pcm_uframes_t frames;
    unsigned int rate;
    int channel;
    int dir;
    unsigned int val;
    int size;

    void init(char* hw, snd_pcm_format_t format, int rate, int channel);
};