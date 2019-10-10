#include "connection.h"

/* standard headers */
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <unistd.h>
#include <string>
#include <thread>
#include <vector>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <alsa/asoundlib.h>
#include <sys/reboot.h>

#include <signal.h>

#include <sstream>
template <typename T>
std::string to_string(const T& value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}

#include <utils.h>

/* mraa headers */
#include "mraa/common.hpp"
#include "mraa/gpio.hpp"
#include "mraa/pwm.hpp"

#include "tcpClient/tcpClient.h"
#include "udpBroadcast/udpBroadcast.h"
#include "tcpServer/tcpServer.h"

/* gpio declaration */
#define GPIO_LED_GREEN 13 /* Green*/
#define GPIO_LED_YELLOW 45 /* Yellow */
#define GPIO_LED_RED 46 /* Red */
#define GPIO_BUTTON 12 /* Button */

#define SERVO_V 0
#define SERVO_H 1

#define PWM_SERVO_V 18 /* Vertical */
#define PWM_SERVO_H 19 /* Horizontal */

#define TCP_SERVER_PORT 9090
#define SERVER_BROADCAST_PORT 55123
#define BROADCAST_DURATION 60000 /* ms */
#define CONFIG_FILE ".ipcam.config"

#define MAX_RETRY 5
#define RESET_BUTTON_PRESS_MS 10000

#define DEFAULT_VIDEO_DEVICE "/dev/video0"
#define BACKUP_VIDEO_DEVICE "/dev/video1"
#define DEFAULT_VIDEO_FPS 30

#define DEFAULT_AUDIO_LEFT "plughw:1,0"
#define DEFAULT_AUDIO_RIGHT "plughw:2,0"
#define DEFAULT_AUDIO_LEFT_ID 1
#define DEFAULT_AUDIO_RIGHT_ID 2
#define DEFAULT_AUDIO_DEVICE_NAME "USB PnP Sound Device"

#define SERVO_ANGLE_MIN 0
#define SERVO_ANGLE_MAX 180
#define SERVO_PEROID 20
#define SERVO_MOVE_STEP 0.0002

#define DUTY_CYCLE_MIN 0.5
#define DUTY_CYCLE_MAX 2.5

using namespace std;

struct config cfg;
mraa::Gpio *gpio_led_green = NULL;
mraa::Gpio *gpio_led_yellow = NULL;
mraa::Gpio *gpio_led_red = NULL;
mraa::Gpio *gpio_button = NULL;

mraa::Pwm *pwm_servo_v = NULL;
mraa::Pwm *pwm_servo_h = NULL;

thread *broadcastThread = NULL;

TcpServer *server;
bool wait = true;
int targetId = -1, settingStage = 0;
int errorFlag = 0;
int doneFlag = 0;
int retryCtn = 0;

float currentVAngle = 0.075f;
float currentHAngle = 0.075f;

string msg_ok = "OK",
    msg_error = "ERROR",
    msg_stage_error = "STAGE_ERROR",
    msg_info = "INFO",
    msg_video_device = "VIDEO_DEVICE",
    msg_x_res = "X_RES",
    msg_y_res = "Y_RES",
    msg_fps = "FPS",
    msg_audio_device_1 = "AUDIO_DEVICE_1",
    msg_audio_device_2 = "AUDIO_DEVICE_2";

volatile sig_atomic_t flag = 1;
TcpClient *controlClient = NULL;

void reportAngle(int servo);

void blinkAsc(int id, int ns, int n)
{
    thread *blinkThread = new thread(blink, id, ns, n);
}

void blink(int id, int ns, int n=1)
{
    for(int i = 0; i < n; i++)
    {
        setLed(id, HIGH);
        usleep(ns);
        setLed(id, LOW);
        usleep(ns);
    }
}

void setLed(int id, int status)
{
    if(status != HIGH && status != LOW)
    {
        return;
    }

    switch (id)
    {
    case LED_GREEN:
        gpio_led_green->write(status);
        break;
    case LED_RED:
        gpio_led_red->write(status);
    case LED_YELLOW:
        gpio_led_yellow->write(status);
    default:
        break;
    }
}

void markError();

float get_angle(float angle)
{
    if(angle > SERVO_ANGLE_MAX || angle < SERVO_ANGLE_MIN)
    {
        angle = (SERVO_ANGLE_MAX + SERVO_ANGLE_MIN) / 2;
    }
    float d1 = angle - SERVO_ANGLE_MIN;
    float d2 = SERVO_ANGLE_MAX - SERVO_ANGLE_MIN;
    float ratio = d1 / d2;
    float d3 = DUTY_CYCLE_MAX - DUTY_CYCLE_MIN;
    float result = DUTY_CYCLE_MIN + (d3*ratio);
    return result / SERVO_PEROID;
}

float get_angle_inv(float pwm_data)
{
    float dx = pwm_data * SERVO_PEROID;
    float d3 = DUTY_CYCLE_MAX - DUTY_CYCLE_MIN;
    float ratio = (dx-DUTY_CYCLE_MIN) / d3;
    float d2 = SERVO_ANGLE_MAX - SERVO_ANGLE_MIN;
    float d1 = ratio * d2;
    return d1 + SERVO_ANGLE_MIN;
}

void vServeoTo(float angle)
{
    angle = angle + 90;
    float target = get_angle(angle);
    if(target - currentVAngle > 0)
    {
        while(target - currentVAngle > 0)
        {
            currentVAngle = currentVAngle - SERVO_MOVE_STEP;
            pwm_servo_v->write(currentVAngle);
            reportAngle(SERVO_V);
            usleep(5000);
        }
    }
    else if(target - currentVAngle < 0)
    {
        while(target - currentVAngle < 0)
        {
            currentVAngle = currentVAngle + SERVO_MOVE_STEP;
            pwm_servo_v->write(currentVAngle);
            reportAngle(SERVO_V);
            usleep(5000);
        }
    }
}

void hServeoTo(float angle)
{
    angle = angle + 90;
    cout << "ANGLE " << angle << endl;
    float target = get_angle(angle);
    cout << "TARGET " << target << endl;
    if(target - currentHAngle > 0)
    {
        while(target - currentHAngle > 0)
        {
            currentHAngle = currentHAngle + SERVO_MOVE_STEP;
            pwm_servo_h->write(currentHAngle);
            reportAngle(SERVO_H);
            usleep(5000);
        }
    }
    else if(target - currentHAngle < 0)
    {
        while(target - currentHAngle < 0)
        {
            currentHAngle = currentHAngle - SERVO_MOVE_STEP;
            pwm_servo_h->write(currentHAngle);
            reportAngle(SERVO_H);
            usleep(5000);
        }
    }
}

void sig_handler(int signum)
{
    if (signum == SIGINT) {
        std::cout << "Exiting..." << std::endl;
        if(server!=NULL)
        {
            server->closeServer();
        }
        exit(0);
    }
}

void markError()
{
    gpio_led_red->write(1);
    long long pressTime = -1;
    while(true)
    {
        if(gpio_button->read() == 1)
        {
            if(pressTime == -1)
            {
                pressTime = getTime();
            }
            else
            {
                if(getTime() - pressTime >= RESET_BUTTON_PRESS_MS) {
                    break;
                }
            }            
        }
        else
        {
            pressTime = -1;
        }
        
    }
    gpio_led_red->write(0);
    while(gpio_button->read() == 1) { }
    cout << "reboot" << endl;
    sync();
    reboot(RB_AUTOBOOT);
}

inline bool is_file_exists (const string& name) {
    return ( access( name.c_str(), F_OK ) != -1 );
}

bool loadSetting()
{
    ifstream infile(CONFIG_FILE);

    string line;
    while (getline(infile, line))
    {
        break;
    }

    size_t pos = 0, iter = 0, finish = 0;
    string delimiter = " ";
    std::string token;
    while ((pos = line.find(delimiter)) != std::string::npos+1) {
        token = line.substr(0, pos);

        if(token.compare("") == 0) break;

        switch (iter)
        {
        case 0:
            // videoDevice
            cfg.videoDevice = token;
            break;
        case 1:
            cfg.xres = atoi(token.c_str());
            break;
        case 2:
            cfg.yres = atoi(token.c_str());
            break;
        case 3:
            cfg.fps = atoi(token.c_str());
            break;
        case 4:
            cfg.audioDevice1 = token;
            break;
        case 5:
            cfg.audioDevice2 = token;
            break;
        case 6:
            cfg.videoUrl = token;
            break;
        case 7:
            cfg.audioUrl1 = token;
            break;
        case 8:
            cfg.audioUrl2 = token;
            break;
        case 9:
            cfg.controlUrl = token;
            finish = 1;
            break;
        default:
            break;
        }

        if(finish)
        {
            break;
        }

        line.erase(0, pos + delimiter.length());
        iter++;
    }

    if(iter != 9 || !finish)
    {
        
        return false;
    }
    else
    {
        return true;
    }
}

bool startBroadcast = true;
void broadcast(int ms)
{
    if(broadcastThread == NULL) 
    {
        startBroadcast = true;
        broadcastThread = new thread([ms](){
            vector<string> addrs = UdpBroadcast::getAddrs();
            vector<UdpBroadcast*> ubs;
            for(int i = 0; i < addrs.size(); i++)
            {
                ubs.push_back(new UdpBroadcast(SERVER_BROADCAST_PORT, (char*) addrs[i].c_str()));
            }
            
            string message = "IP_CAM_PORT:" + to_string(TCP_SERVER_PORT);

            long long startTime = getTime();
            cout << "start broadcast" << endl;
            while(startBroadcast)
            {
                cout << "broadcasting..." << endl;
                for(int i = 0; i < ubs.size(); i++)
                {
                    ubs[i]->sendMsg((char *) message.c_str(), message.size());
                }
                // usleep(1000000);
                blink(LED_YELLOW, 500000);
                if(getTime() - startTime >= ms)
                {
                    startBroadcast = false;
                }
            }
            startBroadcast = true;
            broadcastThread = NULL;
            cout << "broadcast finish" << endl;

            if(targetId == -1)
            {
                if(server != NULL)
                {
                    server->closeServer();
                    cout << "Video Server Timeout." << endl;
                }
                markError();
            }
        });
    }
}

void init()
{
    signal(SIGINT, sig_handler);
    mraa::Result status;
    
    gpio_led_green = new mraa::Gpio(GPIO_LED_GREEN);
    gpio_led_yellow = new mraa::Gpio(GPIO_LED_YELLOW);
    gpio_led_red = new mraa::Gpio(GPIO_LED_RED);
    gpio_button = new mraa::Gpio(GPIO_BUTTON);

    pwm_servo_h = new mraa::Pwm(PWM_SERVO_H);
    pwm_servo_v = new mraa::Pwm(PWM_SERVO_V);

    /* set GPIO to output */
    status = gpio_led_green->dir(mraa::DIR_OUT);
    if (status != mraa::SUCCESS) {
        printError(status);
    }

    status = gpio_led_yellow->dir(mraa::DIR_OUT);
    if (status != mraa::SUCCESS) {
        printError(status);
    }

    status = gpio_led_red->dir(mraa::DIR_OUT);
    if (status != mraa::SUCCESS) {
        printError(status);
    }

    status = gpio_button->dir(mraa::DIR_IN);
    if (status != mraa::SUCCESS) {
        printError(status);
    }

    pwm_servo_h->period_ms(SERVO_PEROID);
    pwm_servo_v->period_ms(SERVO_PEROID);

    status = pwm_servo_h->enable(true);
    if (status != mraa::SUCCESS) {
        printError(status);
    }

    status = pwm_servo_v->enable(true);
    if (status != mraa::SUCCESS) {
        printError(status);
    }

    // // To Zero
    // pwm_servo_h->write(0.075);
    // usleep(5000000);
    // pwm_servo_v->write(0.075);
    // usleep(5000000);

    currentVAngle = 0.075f;
    currentHAngle = 0.075f;
}

string loadVideoDevice(string name)
{
    int fd;
    struct v4l2_capability video_cap;
    struct v4l2_format     video_fmt;

    if((fd = open(name.c_str(), O_RDONLY)) == -1){
        perror("cam_info: Can't open device");
        return "cam_info: Can't open device";
    }

    if(ioctl(fd, VIDIOC_QUERYCAP, &video_cap) == -1)
    {
        perror("cam_info: Can't get capabilities");
        return "cam_info: Can't get capabilities";
    }
    else
    {
        cout << "video device " << name << " go : " << video_cap.card << endl;
        cfg.videoDevice = string(name);
    }

    video_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(fd, VIDIOC_G_FMT, &video_fmt) == -1)
    {
        perror("cam_info: Can't get window information");
        return "cam_info: Can't get window information";
    }
    else
    {
        cout << "video device " << name << " resolution go : ";
        printf("size: %d x %d\n", video_fmt.fmt.pix.width, video_fmt.fmt.pix.height);
        cfg.xres = video_fmt.fmt.pix.width;
        cfg.yres = video_fmt.fmt.pix.height;
    }

    cfg.fps = DEFAULT_VIDEO_FPS;

    close(fd);
    return "";
}

string loadAudioDevice(int id, string deviceName)
{
    char *dn;
    int rc;
    rc = snd_card_get_name (id, &dn);
    if(rc < 0)
    {
        string msg = "audio device " + to_string(id) + " error : " + snd_strerror(rc);
        cout << msg << endl;
        return msg;
    }

    if(string(dn).compare(deviceName) == 0)
    {
        cout << "audio device " << id << " go : " << dn << endl;
    }
    else
    {
        string msg = "audio device " + to_string(id) + " error : " + "device name not match";
        cout << msg << endl;
        return msg;
    }
    
    return "";
}

string loadDevices()
{
    string result = loadVideoDevice(DEFAULT_VIDEO_DEVICE);
    if(result.compare("") != 0)
    {
        result = loadVideoDevice(BACKUP_VIDEO_DEVICE);
    }

    if(result.compare("") != 0)
    {
        return result;
    }

    result = loadAudioDevice(DEFAULT_AUDIO_LEFT_ID, DEFAULT_AUDIO_DEVICE_NAME);
    if(result.compare("") != 0)
    {
        return result;
    }
    else
    {
        cfg.audioDevice1 = DEFAULT_AUDIO_LEFT;
    }

    result = loadAudioDevice(DEFAULT_AUDIO_RIGHT_ID, DEFAULT_AUDIO_DEVICE_NAME);
    if(result.compare("") != 0)
    {
        return result;
    }
    else
    {
        cfg.audioDevice2 = DEFAULT_AUDIO_RIGHT;
    }

    cout << "go for launch" << endl;
    
    return "";
}

bool closeFlag = false;
void onClose()
{
    cout << "CLOSE" << endl;
    if(closeFlag) return;
    closeFlag = true;
    if(errorFlag > 0)
    {
        server->closeServer();
        markError();
        return;
    }

    if(doneFlag > 0)
    {
        server->closeServer();
        wait = false;
        return;
    }
    
    if(retryCtn > MAX_RETRY)
    {
        broadcast(BROADCAST_DURATION);
        cout << "video server connection error, re-try..." << endl;
        server->setAccept(true);
    }
    else
    {
        server->closeServer();
        markError();
    }
    retryCtn++;
}

void saveCfg()
{
    ofstream myfile;
    myfile.open (CONFIG_FILE);
    myfile << cfg.videoDevice << " " << cfg.xres << " " << cfg.yres << " " << cfg.fps << " " << cfg.audioDevice1 << " " << cfg.audioDevice2 << " " << cfg.videoUrl << " " << cfg.audioUrl1 << " " << cfg.audioUrl2 << " " << cfg.controlUrl << endl;
    myfile.close();
}

string controlVStr = "V_ANG:";
string controlHStr = "H_ANG:";
void handleServerCommand()
{
    if(controlClient == NULL)
    {
        cout << "CREATE CONTROL URL" << endl;
        controlClient = new TcpClient(cfg.controlUrl);
    }
    
    controlClient->setOnMessage([](struct sockaddr_in* addr, int id, std::string message){
        int pos;
        if((pos = message.find(controlVStr)) != std::string::npos)
        {
            size_t pos = message.find(controlVStr);
            string data = message.substr(pos+controlVStr.size());
            float angle = atof(data.c_str());

            std::cout << "server CMD : vertical servo angle change to : " << angle << endl;
            vServeoTo(angle);
            server->sendMsg(id, msg_ok);
        } else if((pos = message.find(controlHStr)) != std::string::npos)
        {
            size_t pos = message.find(controlHStr);
            string data = message.substr(pos+controlHStr.size());
            float angle = atof(data.c_str());

            std::cout << "server CMD : horizontal servo angle change to : " << angle << endl;
            hServeoTo(angle);
            server->sendMsg(id, msg_ok);
        }
    });    
    controlClient->sendMsg("OK");
    reportAngle(SERVO_V);
    reportAngle(SERVO_H);
}

void reportAngle(int servo)
{
    if(controlClient == NULL) return;
    float angle;
    string msg;
    if(servo == SERVO_V)
    {
        msg = controlVStr;
        angle = get_angle_inv(currentVAngle) - 90;
    }
    else if(servo == SERVO_H)
    {
        msg = controlHStr;
        angle = get_angle_inv(currentHAngle) - 90;
    }
    else
    {
        return;
    }
    
    controlClient->sendMsg(msg + to_string(angle));
}

struct config *_connectToServer()
{
    cout << "start init gpio" << endl;
    init();
    cout << "go" << endl;

    cout << "checking configuration" << endl;
    if(is_file_exists(CONFIG_FILE))
    {
        if(loadSetting())
        {
            cout << "setting loaded" << endl;
            printf("videoDevice=%s,\naudioDevice1=%s,\naudioDevice2=%s,\nvideoUrl=%s,\naudioUrl1=%s,\naudioUrl2=%s,\ncontrolUrl=%s,\nxres = %d,\nyres = %d,\nfps = %d\n", 
            cfg.videoDevice.c_str(), cfg.audioDevice1.c_str(), cfg.audioDevice2.c_str(), cfg.videoUrl.c_str(), cfg.audioUrl1.c_str(), cfg.audioUrl2.c_str(), cfg.controlUrl.c_str(), cfg.xres, cfg.yres, cfg.fps);
            return &cfg;
        }
        else
        {
            cout << "setting incorrect" << endl;
        }
    }
    else
    {
        cout << "setting not found" << endl;
    }


    cout << "trying to connect to server" << endl;
    broadcast(BROADCAST_DURATION);

    server = new TcpServer(TCP_SERVER_PORT);
    
    server->setAccept(true);

    server->setOnConnection([](struct sockaddr_in * client, int id){
        startBroadcast = false;
        char *paddr_str = inet_ntoa(client->sin_addr); 
        cout << "new client [" << paddr_str << "] connected" << endl;
        targetId = id;
        server->setAccept(false);
        char hellow[] = "HELLO";
        server->sendMsg(id, hellow, sizeof(hellow)-1);
    });

    server->setOnClose([](struct sockaddr_in * client, int id){ 
        if(id != targetId) return;
        onClose();
    });

    server->setOnMessage([](struct sockaddr_in * client, int id, std::string message){
        if(doneFlag) return;
        if(targetId != id)
        {
            server->closeClient(id);
            server->setAccept(true);
        }

        string tag_v_url = "V_URL:";
        string tag_a_url_1 = "A_URL_1:";
        string tag_a_url_2 = "A_URL_2:";
        string tag_ctl_url = "CTL_URL:";

        if(settingStage == 0 && message.compare("INFO") == 0)
        {
            cout << "loading device" << endl;
            string result = loadDevices();
            if(result.compare("") == 0)
            {
                string msg = msg_info+"\n";
                msg += msg_video_device+":"+cfg.videoDevice+"\n";
                msg += msg_x_res+":"+to_string(cfg.xres)+"\n";
                msg += msg_y_res+":"+to_string(cfg.yres)+"\n";
                msg += msg_fps+":"+to_string(cfg.fps)+"\n";
                msg += msg_audio_device_1+":"+to_string(cfg.audioDevice1)+"\n";
                msg += msg_audio_device_2+":"+to_string(cfg.audioDevice2)+"\n";
                msg += msg_ok;
                server->sendMsg(id, msg);
                settingStage++;
            }
            else
            {
                errorFlag = 1;
                server->sendMsg(id, msg_error+":"+result);
                server->closeClient(id);
            }            
        }
        else if(settingStage == 1 && message.find(tag_v_url) != std::string::npos)
        {
            size_t pos = message.find(tag_v_url);
            string url = message.substr(pos+tag_v_url.size());

            std::cout << "video url : " << url << endl;
            cfg.videoUrl = url;
            server->sendMsg(id, msg_ok);
            settingStage++;
        }
        else if(settingStage == 2 && message.find(tag_a_url_1) != std::string::npos)
        {
            size_t pos = message.find(tag_a_url_1);
            string url = message.substr(pos+tag_a_url_1.size());

            cout << "audio url 1 : " << url << endl;
            cfg.audioUrl1 = url;
            server->sendMsg(id, msg_ok);
            settingStage++;
        }
        else if(settingStage == 3 && message.find(tag_a_url_2) != std::string::npos)
        {
            size_t pos = message.find(tag_a_url_2);
            string url = message.substr(pos+tag_a_url_2.size());

            cout << "audio url 2 : " << url << endl;
            cfg.audioUrl2 = url;
            server->sendMsg(id, msg_ok);
            settingStage++;
        }
        else if(settingStage == 4 && message.find(tag_ctl_url) != std::string::npos)
        {
            size_t pos = message.find(tag_ctl_url);
            string url = message.substr(pos+tag_ctl_url.size());

            cout << "control url : " << url << endl;
            cfg.controlUrl = url;
            server->sendMsg(id, msg_ok);
            settingStage++;
        }
        else if(settingStage == 5 && message.find("DONE") != std::string::npos)
        {
            cout << "config finish" << endl;
            server->sendMsg(id, msg_ok);
            doneFlag = 1;
            server->closeClient(id);
            onClose();
        }
        else
        {
            server->sendMsg(id, msg_stage_error);
        }        
    });

    server->start();
    cout << "waiting for video server connection" << endl;
    while(wait) { }
    cout << "DONE" << endl;

    saveCfg();
    cout << "config saved" << endl;

    return &cfg;
}

struct config *connectToServer()
{
    struct config *_cfg = _connectToServer();
    cout << "handling server command" << endl;
    handleServerCommand();
    return _cfg;
}