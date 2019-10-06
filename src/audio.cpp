#include "audio.h"
#include "utils.h"

#include <sstream>
template <typename T>
std::string to_string(const T& value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}

Audio::Audio(string device)
{
    this->init(device);
}

void Audio::init(string device)
{
    this->run = true;
    recorder = new AlsaRecord((char *) device.c_str());
}

void Audio::start(string tcpUrl, int chunkSize)
{
    this->recordThread = new thread( &Audio::record, this, tcpUrl, chunkSize );
}

void Audio::stop()
{
    this->run = false;
    this->recordThread->join();
}

void Audio::setId(int id)
{
    this->id = id;
}

int Audio::getId()
{
    return this->id;
}

void Audio::srart(int chunkSize, vector<char> *audio)
{
    int size = recorder->getBufferSize();
    int loops = chunkSize / recorder->getPeriod();

    while(loops--)
    {
        char buffer[size];
        size = recorder->recordDirect(buffer, size);
        this->size = size;
        audio->insert(audio->end(), buffer, buffer+size);
    }
}



void Audio::record(string tcpUrl, int chunkSize)
{
    TcpClient client(tcpUrl);

    vector<char> audio;
    
    int size = recorder->getBufferSize();
    int loops = chunkSize / recorder->getPeriod();

    int sid = 0;
    int lastId = 0;
    while(this->run)
    {
        char buffer[size];
        size = recorder->recordDirect(buffer, size);
        this->size = size;
        audio.insert(audio.end(), buffer, buffer+size);

        if(loops-- == 0) {

            char start[] = "-2\n-2\n";
            client.sendMsg(start, sizeof(start)-1);

            if(id != -1)
            {
                if(id != lastId) {
                    sid = 0;
                    lastId = id;
                }

                char tag[] = "id:";
                client.sendMsg(tag, sizeof(tag)-1);

                string s_id = to_string(id);
                client.sendMsg((char*)s_id.c_str(), s_id.size());
                
                char sep[] = "-3\n-3\n";
                client.sendMsg(sep, sizeof(sep)-1);

                char tag_sid[] = "sid:";
                client.sendMsg(tag_sid, sizeof(tag_sid)-1);

                string s_sid = to_string(sid);
                client.sendMsg((char*)s_sid.c_str(), s_sid.size());
                sid++;

                client.sendMsg(sep, sizeof(sep)-1);
            }

            int code = client.sendMsg(&audio[0], audio.size());

            char eof[] = "-1\n-1\n";
            client.sendMsg(eof, sizeof(eof)-1);
            audio.clear();
            loops = chunkSize / recorder->getPeriod();
        }        
    }    
}