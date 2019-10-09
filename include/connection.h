#include <string>

#define LED_RED 0
#define LED_YELLOW 1
#define LED_GREEN 2

#define HIGH 1
#define LOW 0

using namespace std;

struct config
{
    string videoDevice, audioDevice1, audioDevice2;
    string videoUrl, audioUrl1, audioUrl2, controlUrl;
    int xres = -1, yres = -1, fps = -1;
};

struct config *connectToServer();

void vServeoTo(float angle);
void hServeoTo(float angle);
void blinkAsc(int id, int ns, int n);
void blink(int id, int ns, int n);
void setLed(int id, int status);