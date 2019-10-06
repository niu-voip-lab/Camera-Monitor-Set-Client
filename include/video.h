#include "iostream"
#include "cstdlib"
#include "string"
#include "thread"

#include "linux/videodev2.h"
#include "inc/V4l2Capture.h"
#include "tcpClient/tcpClient.h"

using namespace std;

class Video
{
public:
    Video(string device, int resX, int resY, int fps);
    size_t start(char* buffer, size_t bufferSize);
    void start(string tcpUrl);
    void stop();
    int getFps();
    void setId(int id);
    int getId();
private:
    string device;
    int resX, resY, fps, realFps;
    V4l2Capture* videoCapture;
    bool run;
    thread *recordThread;
    int id = -1;
    
    void init(string device, int resX, int resY, int fps);
    void record(string tcpUrl);
};