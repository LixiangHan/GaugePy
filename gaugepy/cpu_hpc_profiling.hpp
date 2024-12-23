#ifndef _HPP_CPU_HPC_PROFILING_
#define _HPP_CPU_HPC_PROFILING_

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <errno.h>
#include <linux/hw_breakpoint.h>
#include <linux/perf_event.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

namespace py = pybind11;

struct CpuPerfEvent
{
    uint32_t type;
    uint64_t config;
    std::string name;
};

struct CpuEventGroups
{
    uint32_t num_groups;
    std::vector<uint32_t> num_group_events;
    std::vector<std::vector<uint32_t>> types;
    std::vector<std::vector<uint64_t>> configs;
};

std::map<std::string, float> cpu_hpc_profiling(py::function callback, const std::vector<std::string> &metricNames);

std::vector<std::string> available_cpu_metrics();

#endif
