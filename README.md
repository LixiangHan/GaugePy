# GaugePy
A highly user-friendly Python library for measuring hardware performance counters on both CPU and GPU.

## Prerequisites

- [Linux Perf tool](https://perfwiki.github.io/main/)
- Nivdia GPU with capability >= 70

## Installation
```
git clone https://github.com/LixiangHan/GaugePy.git
cd GaugePy
pip install -e gaugepy
```

## Usage

### CPU HPC Profiling
```python
import gaugepy

def test():
    a = 0
    for i in range(1000):
        a += i

available_cpu_metrics = gaugepy.available_cpu_metrics()

print(available_cpu_metrics)

print(gaugepy.cpu_hpc_profiling(test, available_cpu_metrics))
```

### GPU HPC Profiling
```python
import gaugepy
import torch

def test():
    x = torch.randn(1000, 1000, device="cuda")
    y = torch.randn(1000, 1000, device="cuda")
    z = x + y
    return z

print(gaugepy.gpu_hpc_profiling(test, ["smsp__warps_launched.avg"]))
```

## Citation

If you find GaugePy useful in your research, please consider citing:
```bibtex
@misc{gaugepy,
  author = {Lixiang Han},
  title = {GaugePy},
  year = {2024},
  publisher = {GitHub},
  journal = {GitHub repository},
  howpublished = {\url{https://github.com/LixiangHan/GaugePy}},
}
```

## Disclaimer

This library is not fully tested and may not work on all platforms. If you find any issues, feel free to contribute to this project.
