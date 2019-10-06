#include "cstdlib"
#include "iostream"
#include "string"
#include "vector"
#include "thread"

#include "recordLib/recordLib.h"
#include "tcpClient/tcpClient.h"


#define DEFAULT_CHUNK_SIZE 33000 //micro seconds

using namespace std;

class Audio
{
public:
    Audio(string device);
    void srart(int chunkSize, vector<char> *audio);
    void start(string tcpUrl, int chunkSize);
    void stop();
    void setId(int id);
    int getId();
    int getSize();
private:
    AlsaRecord* recorder;
    thread *recordThread;
    bool run;
    int id = -1;
    int size = 0;

    void init(string device);
    void record(string tcpUrl, int chunkSize);
};