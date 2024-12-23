#include "cpu_hpc_profiling.hpp"

struct read_format
{
    uint64_t nr; /* The number of events */
    struct
    {
        uint64_t value; /* The value of the event */
        uint64_t id;    /* if PERF_FORMAT_ID */
    } values[];
};

static CpuPerfEvent CPU_PERF_EVENTS[] = {
    {PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES, "PERF_COUNT_HW_CPU_CYCLES"},
    {PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS, "PERF_COUNT_HW_INSTRUCTIONS"},
    {PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_REFERENCES, "PERF_COUNT_HW_CACHE_REFERENCES"},
    {PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES, "PERF_COUNT_HW_CACHE_MISSES"},
    {PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_INSTRUCTIONS, "PERF_COUNT_HW_BRANCH_INSTRUCTIONS"},
    {PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_MISSES, "PERF_COUNT_HW_BRANCH_MISSES"},
    {PERF_TYPE_HARDWARE, PERF_COUNT_HW_BUS_CYCLES, "PERF_COUNT_HW_BUS_CYCLES"},

    {PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CPU_CLOCK, "PERF_COUNT_SW_CPU_CLOCK"},
    {PERF_TYPE_SOFTWARE, PERF_COUNT_SW_TASK_CLOCK, "PERF_COUNT_SW_TASK_CLOCK"},
    {PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS, "PERF_COUNT_SW_PAGE_FAULTS"},
    {PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CONTEXT_SWITCHES, "PERF_COUNT_SW_CONTEXT_SWITCHES"},
    {PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CPU_MIGRATIONS, "PERF_COUNT_SW_CPU_MIGRATIONS"},
    {PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS_MIN, "PERF_COUNT_SW_PAGE_FAULTS_MIN"},
    {PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS_MAJ, "PERF_COUNT_SW_PAGE_FAULTS_MAJ"},

    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_L1D) | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16), "L1D_READ_ACCESS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_L1D) | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16), "L1D_READ_MISS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_L1D) | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16), "L1D_WRITE_ACCESS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_L1D) | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16), "L1D_WRITE_MISS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_L1D) | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16), "L1D_PREFETCH_ACCESS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_L1D) | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16), "L1D_PREFETCH_MISS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_L1I) | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16), "L1I_READ_ACCESS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_L1I) | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16), "L1I_READ_MISS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_L1I) | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16), "L1I_WRITE_ACCESS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_L1I) | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16), "L1I_WRITE_MISS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_L1I) | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16), "L1I_PREFETCH_ACCESS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_L1I) | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16), "L1I_PREFETCH_MISS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_LL) | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16), "LL_READ_ACCESS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_LL) | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16), "LL_READ_MISS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_LL) | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16), "LL_WRITE_ACCESS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_LL) | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16), "LL_WRITE_MISS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_LL) | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16), "LL_PREFETCH_ACCESS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_LL) | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16), "LL_PREFETCH_MISS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_DTLB) | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16), "DTLB_READ_ACCESS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_DTLB) | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16), "DTLB_READ_MISS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_DTLB) | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16), "DTLB_WRITE_ACCESS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_DTLB) | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16), "DTLB_WRITE_MISS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_DTLB) | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16), "DTLB_PREFETCH_ACCESS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_DTLB) | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16), "DTLB_PREFETCH_MISS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_ITLB) | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16), "ITLB_READ_ACCESS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_ITLB) | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16), "ITLB_READ_MISS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_ITLB) | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16), "ITLB_WRITE_ACCESS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_ITLB) | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16), "ITLB_WRITE_MISS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_ITLB) | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16), "ITLB_PREFETCH_ACCESS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_ITLB) | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16), "ITLB_PREFETCH_MISS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_BPU) | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16), "BPU_READ_ACCESS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_BPU) | (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16), "BPU_READ_MISS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_BPU) | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16), "BPU_WRITE_ACCESS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_BPU) | (PERF_COUNT_HW_CACHE_OP_WRITE << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16), "BPU_WRITE_MISS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_BPU) | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16), "BPU_PREFETCH_ACCESS"},
    {PERF_TYPE_HW_CACHE, (PERF_COUNT_HW_CACHE_BPU) | (PERF_COUNT_HW_CACHE_OP_PREFETCH << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16), "BPU_PREFETCH_MISS"},
};

static uint32_t NUM_CPU_PERF_EVENTS = sizeof(CPU_PERF_EVENTS) / sizeof(CpuPerfEvent);

static long perf_event_open(struct perf_event_attr *attr, pid_t pid,
                            int cpu, int group_fd, unsigned long flags)
{
    int ret;

    ret = syscall(__NR_perf_event_open, attr, pid, cpu, group_fd, flags);

    return ret;
}

static long perf_event_close(int fd)
{
    if (fd >= 0)
    {
        if (close(fd) == -1)
        {
            printf("%s:%d: failed to close the file descriptor: %s.\n", __FILE__, __LINE__, strerror(errno));
            return -1;
        }
    }
    return 0;
}

static int num_cpu_hw_counters()
{
    FILE *fp;
    char line[1024];
    int count = 0;

    fp = popen("perf list hw | grep -c 'Hardware event'", "r");
    if (fp == NULL)
    {
        printf("%s:%d: failed to execute command.\n", __FILE__, __LINE__);
        return -1;
    }

    if (fgets(line, sizeof(line), fp) != NULL)
    {
        count = atoi(line);
    }

    pclose(fp);
    return count;
}

static int event_name_to_id(const std::string event_name)
{
    for (int i = 0; i < NUM_CPU_PERF_EVENTS; i++)
    {
        if (CPU_PERF_EVENTS[i].name == event_name)
        {
            return i;
        }
    }
    return -1;
}

std::map<std::string, float> cpu_hpc_profiling(py::function callback, const std::vector<std::string> &metricNames)
{
    std::map<std::string, float> metricValues;
    std::vector<int> metricId;
    CpuEventGroups cpu_event_groups;
    int num_hw_counters = num_cpu_hw_counters() - 1; // One is for cycle counter only. See https://www.ee.torontomu.ca/~courses/coe838/Data-Sheets/Cortex_A57_MPcore.pdf
    int num_hw_events = 0;
    int num_sw_events = 0;
    for (int i = 0; i < metricNames.size(); i++)
    {
        int event_id = event_name_to_id(metricNames[i]);
        if (event_id == -1)
        {
            printf("%s:%d: event name not found: %s.\n", __FILE__, __LINE__, metricNames[i].c_str());
            exit(-1);
        }
        metricId.push_back(event_id);
        if (CPU_PERF_EVENTS[event_id].type == PERF_TYPE_SOFTWARE)
            num_sw_events++;
        else
            num_hw_events++;
    }

    cpu_event_groups.num_groups = (num_hw_events + num_hw_counters - 1) / num_hw_counters; // ceiling of num_hw_events / num_hw_counters
    cpu_event_groups.num_group_events.resize(cpu_event_groups.num_groups, 0);
    cpu_event_groups.types.resize(cpu_event_groups.num_groups);
    cpu_event_groups.configs.resize(cpu_event_groups.num_groups);

    int index = 0;
    for (int i = 0; i < cpu_event_groups.num_groups; i++)
    {
        uint32_t event_count = 0;
        uint32_t hw_event_count = 0;
        while (hw_event_count < num_hw_counters && index < metricId.size())
        {
            event_count++;
            if (CPU_PERF_EVENTS[metricId[index]].type != PERF_TYPE_SOFTWARE)
                hw_event_count++;
            index++;
        }
        cpu_event_groups.num_group_events[i] = event_count;
        cpu_event_groups.types[i].resize(cpu_event_groups.num_group_events[i]);
        cpu_event_groups.configs[i].resize(cpu_event_groups.num_group_events[i]);
    }

    index = 0;
    for (int i = 0; i < cpu_event_groups.num_groups; i++)
    {
        for (int j = 0; j < cpu_event_groups.num_group_events[i]; j++)
        {
            cpu_event_groups.types[i][j] = CPU_PERF_EVENTS[metricId[index]].type;
            cpu_event_groups.configs[i][j] = CPU_PERF_EVENTS[metricId[index]].config;
            index++;
        }
    }

    index = 0;
    for (int i = 0; i < cpu_event_groups.num_groups; i++) // cpu_event_groups.num_groups passes is required to profile all specified cpu hpc
    {
        pid_t pid = 0;
        int cpu = -1; // This measures the calling process/thread on any CPU
        struct perf_event_attr pe;

        std::vector<int> fds(cpu_event_groups.num_group_events[i], 0);
        std::vector<uint64_t> ids(cpu_event_groups.num_group_events[i], 0);

        for (int j = 0; j < cpu_event_groups.num_group_events[i]; j++)
        {
            memset(&pe, 0, sizeof(struct perf_event_attr));
            pe.type = cpu_event_groups.types[i][j];
            pe.size = sizeof(struct perf_event_attr);
            pe.config = cpu_event_groups.configs[i][j];
            pe.disabled = 1;
            pe.exclude_kernel = 1; // don't count kernel
            pe.exclude_hv = 1;     // don't count hypervisor
            pe.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;
            if (j == 0)
                fds[j] = perf_event_open(&pe, pid, cpu, -1, 0); // Set this as the group leader
            else
                fds[j] = perf_event_open(&pe, pid, cpu, fds[0], 0);
            if (fds[j] == -1)
            {
                printf("%s:%d: failed to set up the performance counter: %s.\n", __FILE__, __LINE__, strerror(errno));
                exit(-1);
            }

            ioctl(fds[j], PERF_EVENT_IOC_ID, &ids[j]);
        }

        ioctl(fds[0], PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
        ioctl(fds[0], PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP); // Reset and enable the event group

        // ----------------------------------------
        // RUN THE PROGRAM TO BE PROFILED HERE
        // ----------------------------------------
        callback();

        ioctl(fds[0], PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP); // Disable the event group

        std::vector<char> buffer(cpu_event_groups.num_group_events[i] * 16 + 8, 0);

        if (read(fds[0], buffer.data(), buffer.size()) == -1)
        {
            printf("%s:%d: failed to read the performance counter values: %s.\n", __FILE__, __LINE__, strerror(errno));
            exit(-1);
        }

        struct read_format *reads = (struct read_format *)buffer.data();

        for (int j = 0; j < cpu_event_groups.num_group_events[i]; j++)
        {
            for (int k = 0; k < reads->nr; k++)
            {
                if (reads->values[k].id == ids[j])
                {
                    metricValues[CPU_PERF_EVENTS[metricId[index]].name] = reads->values[k].value;
                    index++;
                    break;
                }
            }
        }

        for (int j = 0; j < cpu_event_groups.num_group_events[i]; j++)
        {
            perf_event_close(fds[j]);
        }
    }

    return metricValues;
}

std::vector<std::string> available_cpu_metrics()
{
    std::vector<std::string> metricNames;
    struct perf_event_attr pe;
    for (int i = 0; i < NUM_CPU_PERF_EVENTS; i++)
    {
        memset(&pe, 0, sizeof(struct perf_event_attr));
        pe.type = CPU_PERF_EVENTS[i].type;
        pe.size = sizeof(struct perf_event_attr);
        pe.config = CPU_PERF_EVENTS[i].config;
        pe.disabled = 1;
        pe.exclude_kernel = 1; /* don't count kernel      */
        pe.exclude_hv = 1;     /* don't count hypervisor  */
        pe.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;
        int fd = perf_event_open(&pe, 0, -1, -1, 0);
        if (fd != -1)
        {
            metricNames.push_back(CPU_PERF_EVENTS[i].name);
            perf_event_close(fd);
        }
    }

    return metricNames;
}
