#include "video.h"
#include "utils.h"

#include <sstream>
template <typename T>
std::string to_string(const T& value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}

Video::Video(string device, int resX, int resY, int fps)
{
    this->init(device, resX, resY, fps);
}

void Video::init(string device, int resX, int resY, int fps)
{
    this->device = device;
    this->resX = resX;
    this->resY = resY;
    this->fps = fps;

    this->run = true;

    V4L2DeviceParameters param(device.c_str(), V4L2_PIX_FMT_MJPEG, resX, resY, fps, 1);
    this->videoCapture = V4l2Capture::create(param, V4l2Access::IOTYPE_MMAP);
}

void Video::start(string tcpUrl)
{
    this->recordThread = new thread( &Video::record, this, tcpUrl );
}

void Video::stop()
{
    this->run = false;
    this->recordThread->join();
}

void Video::setId(int id)
{
    this->id = id;
}

int Video::getId()
{
    return this->id;
}

size_t Video::start(char* buffer, size_t bufferSize)
{
    timeval timeout;
    size_t nb = 0;
    while(true)
    {
        bool isReadable = (videoCapture->isReadable(&timeout) == 1);
        if (isReadable)
        {
            nb = videoCapture->read(buffer, bufferSize);
            break;
        }
    }
    return nb;
}


void Video::record(string tcpUrl)
{
    int ctn = 0;
    long long lastTime;

    TcpClient client(tcpUrl);

    int nx=0;

    timeval timeout;

    int bufferSize = this->resX * this->resY;

    int sid = 0;
    int lastId = 0;
    while (this->run)
    {
        bool isReadable = (videoCapture->isReadable(&timeout) == 1);
        if (isReadable)
        {
            char buffer[bufferSize];
            size_t nb = videoCapture->read(buffer, bufferSize);

            char start[] = "-2\n-2\n";
            client.sendMsg(start, sizeof(start)-1);

            if(this->id != -1)
            {
                if(id != lastId) {
                    sid = 0;
                    lastId = id;
                }

                char tag[] = "id:";
                client.sendMsg(tag, sizeof(tag)-1);

                string s_id = to_string(id);
                client.sendMsg((char*)s_id.c_str(), s_id.size());
                
                char start[] = "-3\n-3\n";
                client.sendMsg(start, sizeof(start)-1);

                char tag_sid[] = "sid:";
                client.sendMsg(tag_sid, sizeof(tag_sid)-1);

                string s_sid = to_string(sid);
                client.sendMsg((char*)s_sid.c_str(), s_sid.size());

                client.sendMsg(start, sizeof(start)-1);

                sid++;
            }

            int code = client.sendMsg(buffer, nb);

            char eof[] = "-1\n-1\n";
            client.sendMsg(eof, sizeof(eof)-1);
            ctn++;
        }

        if (getTime() - lastTime >= 1000)
        {
            this->realFps = ctn;
            ctn = 0;
            lastTime = getTime();
        }
    }
}

int Video::getFps()
{
    return this->realFps;
}