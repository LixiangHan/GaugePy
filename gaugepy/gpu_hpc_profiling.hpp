#ifndef _HPP_GPU_HPC_PROFILING_
#define _HPP_GPU_HPC_PROFILING_

#include <cupti_target.h>
#include <cupti_profiler_target.h>
#include <nvperf_host.h>
#include <nvperf_cuda_host.h>
#include <nvperf_target.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <Metric.h>
#include <Eval.h>
#include <Utils.h>
#include <Parser.h>
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#define NVPW_API_CALL(apiFuncCall)                                               \
    do                                                                           \
    {                                                                            \
        NVPA_Status _status = apiFuncCall;                                       \
        if (_status != NVPA_STATUS_SUCCESS)                                      \
        {                                                                        \
            fprintf(stderr, "%s:%d: error: function %s failed with error %d.\n", \
                    __FILE__, __LINE__, #apiFuncCall, _status);                  \
            exit(-1);                                                            \
        }                                                                        \
    } while (0)

#define CUPTI_API_CALL(apiFuncCall)                                              \
    do                                                                           \
    {                                                                            \
        CUptiResult _status = apiFuncCall;                                       \
        if (_status != CUPTI_SUCCESS)                                            \
        {                                                                        \
            const char *errstr;                                                  \
            cuptiGetResultString(_status, &errstr);                              \
            fprintf(stderr, "%s:%d: error: function %s failed with error %s.\n", \
                    __FILE__, __LINE__, #apiFuncCall, errstr);                   \
            exit(-1);                                                            \
        }                                                                        \
    } while (0)

#define DRIVER_API_CALL(apiFuncCall)                                             \
    do                                                                           \
    {                                                                            \
        CUresult _status = apiFuncCall;                                          \
        if (_status != CUDA_SUCCESS)                                             \
        {                                                                        \
            fprintf(stderr, "%s:%d: error: function %s failed with error %d.\n", \
                    __FILE__, __LINE__, #apiFuncCall, _status);                  \
            exit(-1);                                                            \
        }                                                                        \
    } while (0)

#define RUNTIME_API_CALL(apiFuncCall)                                               \
    do                                                                              \
    {                                                                               \
        cudaError_t _status = apiFuncCall;                                          \
        if (_status != cudaSuccess)                                                 \
        {                                                                           \
            fprintf(stderr, "%s:%d: error: function %s failed with error %s.\n",    \
                    __FILE__, __LINE__, #apiFuncCall, cudaGetErrorString(_status)); \
            exit(-1);                                                               \
        }                                                                           \
    } while (0)

static int numRanges = 1;

std::map<std::string, float> gpu_hpc_profiling(py::function callback, const std::vector<std::string> &metricNames);

#endif
