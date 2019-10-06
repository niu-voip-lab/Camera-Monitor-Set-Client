#include <performanceLib.h>

Monotor::Monotor()
{

}

void Monotor::process_mem_usage()
{
    this->vm_usage = 0.0;
    this->resident_set = 0.0;

    // the two fields we want
    unsigned long vsize;
    long rss;
    {
        std::string ignore;
        std::ifstream ifs("/proc/self/stat", std::ios_base::in);
        ifs >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
                >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
                >> ignore >> ignore >> vsize >> rss;
    }

    long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
    this->vm_usage = vsize / 1024.0;
    this->resident_set = rss * page_size_kb;
}


std::vector<size_t> Monotor::get_cpu_times() {
    std::ifstream proc_stat("/proc/stat");
    proc_stat.ignore(5, ' '); // Skip the 'cpu' prefix.
    std::vector<size_t> times;
    for (size_t time; proc_stat >> time; times.push_back(time));
    return times;
}
 
bool Monotor::get_cpu_times(size_t &idle_time, size_t &total_time) {
    const std::vector<size_t> cpu_times = get_cpu_times();
    if (cpu_times.size() < 4)
        return false;
    idle_time = cpu_times[3];
    total_time = std::accumulate(cpu_times.begin(), cpu_times.end(), 0);
    return true;
}

double Monotor::getCpuUsage()
{
    return this->cpu_usage;
}

double Monotor::getVmUsage()
{
    return this->vm_usage;
}

double Monotor::getResidentSet()
{
    return this->resident_set;
}

void Monotor::start()
{
    this->previous_idle_time = this->idle_time;
    this->previous_total_time = this->total_time;
}
 
void Monotor::end()
{
    this->process_mem_usage();
    this->get_cpu_times(this->idle_time, this->total_time);
    const float idle_time_delta = this->idle_time - this->previous_idle_time;
    const float total_time_delta = this->total_time - this->previous_total_time;
    const float utilization = 100.0 * (1.0 - idle_time_delta / total_time_delta);
    this->cpu_usage = utilization;
}