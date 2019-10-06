#include "utils.h"
#include "iostream"
#include "chrono"

long long getTime()
{
    using namespace std::chrono;
    milliseconds ms = duration_cast< milliseconds >(
        system_clock::now().time_since_epoch()
    );
    return ms.count();
}
