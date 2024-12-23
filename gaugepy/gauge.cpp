#include "cpu_hpc_profiling.hpp"
#include "gpu_hpc_profiling.hpp"

PYBIND11_MODULE(gaugepy, m)
{
    m.def("cpu_hpc_profiling", &cpu_hpc_profiling, py::arg("callback"), py::arg("metric_names"));
    m.def("gpu_hpc_profiling", &gpu_hpc_profiling, py::arg("callback"), py::arg("metric_names"));
    m.def("available_cpu_metrics", &available_cpu_metrics);
}