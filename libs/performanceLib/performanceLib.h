#include <fstream>
#include <iostream>
#include <numeric>
#include <unistd.h>
#include <vector>

class Monotor
{
public:
    Monotor();
    void start();
    void end();
    double getCpuUsage();
    double getVmUsage();
    double getResidentSet();
private:
    double vm_usage, resident_set, cpu_usage;
    size_t idle_time=0, total_time=0;
    size_t previous_idle_time=0, previous_total_time=0;
    void process_mem_usage();
    std::vector<size_t> get_cpu_times();
    bool get_cpu_times(size_t &, size_t &);
};