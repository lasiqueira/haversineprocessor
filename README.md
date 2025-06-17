# Haversine Distance processor
Haversine Distance processor written in C++.

Created as homework for Computer Enhance Performance-Aware Porgramming series: https://www.computerenhance.com/p/table-of-contents


## Overview
I'm updating the code trying to make it run multiplatform as much as possible, but ultimately the code will be optimized for x86_64 architecture and my particular machine and OS.

Specs:
```
CPU: AMD Zen 4 R7 7800X3D
Memory: 32GB DDR5 6000Mhz
GPU: NVIDIA RTX 4090
Storage: 2TB NVME SSD Samsung 980 PRO

Windows 11 Pro 24H2
```

## Results

Base Results:
```
File size: 1035913216 bytes
Points: 10000000
Haversine sum: 100101359712.7086486816406250

Total time: 17203.7392ms (timer freq 4199950250)
  ReadPointsJson[1]: 20806716 (0.03%, 30.86% w/children)
  Read file[1]: 22274084088 (30.83%)  987.924mb at 0.18gb/s
  ProcessJson[1]: 48502107991 (67.13%)
  Haversine[1]: 1428547386 (1.98%)  305.176mb at 0.88gb/s
  SumHaversine[1]: 25207434 (0.03%)  76.294mb at 12.41gb/s
```

Results after increasing page size:
```
File size: 1035913216 bytes
Points: 10000000
Haversine sum: 100101359712.7086486816406250

Total time: 12521.7707ms (timer freq 4199988700)
  ReadPointsJson[1]: 226630426 (0.43%, 4.52% w/children)
  Read file[1]: 2151373412 (4.09%)  987.924mb at 1.88gb/s
  ProcessJson[1]: 48837235105 (92.86%)
  Haversine[1]: 1344669437 (2.56%)  305.176mb at 0.93gb/s
  SumHaversine[1]: 25441458 (0.05%)  76.294mb at 12.30gb/s
```
