// #include "iostream"
// #include "cstdlib"
// #include "string"
// #include "thread"
// #include "vector"

// #include "video.h"
// #include "audio.h"
// #include "utils.h"

// using namespace std;

// #define ID_LIFE_TIME 100 //micro-seconds
// #define ID_CIRCLE 100

// template <typename T>
// std::string to_string(const T& value) {
//     std::stringstream ss;
//     ss << value;
//     return ss.str();
// }

// int id = 0;

// void sendBuffer(TcpClient* client, char* buffer, int size, int id);

// int main(int argc, char **argv)
// {
//     string dev(argv[1]);
//     int xres = atoi(argv[2]);
//     int yres = atoi(argv[3]);
//     int fps = atoi(argv[4]);
//     string audioDevice = argv[5];
//     string audioDevice2 = argv[6];
//     string vUrl(argv[7]);
//     string aUrl(argv[8]);
//     string aUrl2(argv[9]);

//     Video video(dev, xres, yres, fps);
//     Audio audio(audioDevice);
//     Audio audio2(audioDevice2);

//     TcpClient* videoClient = new TcpClient(vUrl);
//     TcpClient* audio1Client = new TcpClient(aUrl);
//     TcpClient* audio2Client = new TcpClient(aUrl2);

//     int videoBufferSize = xres*yres;
//     vector<char> audio1Buffer;
//     vector<char> audio2Buffer;
//     float ratio = (1/(float)fps) * 1000000;
//     int chunkSize = ratio;


//     while(true)
//     {
//         char* videoBuffer = (char*) malloc(sizeof(char)*videoBufferSize);
//         thread audio1Thread(&Audio::srart, audio, chunkSize, &audio1Buffer);
//         thread audio2Thread(&Audio::srart, audio2, chunkSize, &audio2Buffer);
//         int videoSize = video.start(videoBuffer, videoBufferSize);
//         audio1Thread.join();
//         audio2Thread.join();

//         sendBuffer(videoClient, videoBuffer, videoBufferSize, 0);
//         sendBuffer(audio1Client, &audio1Buffer[0], audio1Buffer.size(), 0);
//         sendBuffer(audio2Client, &audio2Buffer[0], audio1Buffer.size(), 0);
//         audio1Buffer.clear();
//         audio2Buffer.clear();
//     }


//     return 0;
// }

// void sendBuffer(TcpClient* client, char* buffer, int size, int id)
// {
//     char start[] = "-2\n-2\n";
//     char sep[] = "-3\n-3\n";
//     char eof[] = "-1\n-1\n";

//     client->sendMsg(start, sizeof(start)-1);

//     if(id != -1)
//     {
//         string s_id = to_string(id);
//         client->sendMsg((char*)s_id.c_str(), s_id.size());        
//         client->sendMsg(sep, sizeof(sep)-1);
//     }

//     int code = client->sendMsg(buffer, size);
//     client->sendMsg(eof, sizeof(eof)-1);
// }