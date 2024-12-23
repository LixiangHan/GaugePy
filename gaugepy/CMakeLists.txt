cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(gaugepy LANGUAGES CXX)

find_package(CUDA REQUIRED)
find_package(pybind11 REQUIRED)
find_package(Python COMPONENTS Development REQUIRED)

set(CUDA_INSTALL_PATH /usr/local/cuda)
set(CUPTI_PATH ${CUDA_INSTALL_PATH}/extras/CUPTI)

include_directories(
    ${CUDA_INCLUDE_DIRS}
    ${PYTHON_INCLUDE_DIRS}
    ${pybind11_INCLUDE_DIRS}
    ${CUPTI_PATH}/samples/extensions/include/c_util
    ${CUPTI_PATH}/samples/extensions/include/profilerhost_util
)

add_library(cupti_utils STATIC
    # ${CUPTI_PATH}/samples/extensions/include/c_util/FileOp.h
    # ${CUPTI_PATH}/samples/extensions/include/c_util/ScopeExit.h
    ${CUPTI_PATH}/samples/extensions/src/profilerhost_util/Metric.cpp
    ${CUPTI_PATH}/samples/extensions/src/profilerhost_util/Eval.cpp
    ${CUPTI_PATH}/samples/extensions/src/profilerhost_util/List.cpp
)

link_directories(
    ${CUDA_LIBRARY_DIRS}
    ${CUPTI_PATH}/samples/extensions/src/profilerhost_util
    ${CUPTI_PATH}/lib64
    ${CUDA_INSTALL_PATH}/lib64
)

pybind11_add_module(gaugepy MODULE gpu_hpc_profiling.cpp gpu_hpc_profiling.hpp cpu_hpc_profiling.cpp cpu_hpc_profiling.hpp gauge.cpp)

target_link_libraries(gaugepy PUBLIC ${CUDA_LIBRARIES} cuda cupti nvperf_host nvperf_target Python::Python cupti_utils)
