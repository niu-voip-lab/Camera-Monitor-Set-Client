#include <string>

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