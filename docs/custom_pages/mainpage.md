@mainpage Flex Net Sim

[![Static Badge](https://gitlab.com/DaniloBorquez/flex-net-sim/badges/master/pipeline.svg)](https://gitlab.com/DaniloBorquez/flex-net-sim/-/pipelines)
[![Static Badge](https://img.shields.io/badge/version-0.9.1-blue)](https://gitlab.com/DaniloBorquez/flex-net-sim)
[![Static Badge](https://gitlab.com/DaniloBorquez/flex-net-sim/badges/master/coverage.svg)](https://daniloborquez.gitlab.io/flex-net-sim/coverage-details/)
![Static Badge](https://img.shields.io/badge/language-C%2B%2B-blue)
[![Static Badge](https://img.shields.io/badge/licese-MIT-green)](https://gitlab.com/DaniloBorquez/flex-net-sim/-/blob/master/LICENSE.md?ref_type=heads)
[![Static Badge](https://img.shields.io/badge/DOI-10.48550%2FarXiv.2105.02762-%234682B4
)](https://doi.org/10.48550/arXiv.2105.02762)

**Flex Net Sim** is a C++ simulation library for developing allocation algorithms in flexible grid optical networks. It supports Elastic Optical Networks (EONs), Multi-Band EONs (MB-EONs), and Spatial Division Multiplexing EONs (SDM-EONs), enabling flexible network customization. Using a Poisson-based traffic model, it lets researchers focus solely in the algorithm codification without handling the simulation infrastructure.

Check out our pre-print on <a href="https://arxiv.org/abs/2105.02762" target="_blank" rel="noopener noreferrer">arXiv</a>, and if you find our library useful, please cite us! 😍
```
@misc{borquez2021,
  author        = {Felipe Falcón and Gonzalo España and Danilo Bórquez-Paredes},
  title         = {Flex Net Sim: A Lightweight Manual},
  year          = {2021},
  eprint        = {2105.02762},
  archivePrefix = {arXiv},
  primaryClass  = {cs.NI}
}
```

## Features
- Flexibility to implement custom allocation algorithms for EONs, SDM-EONs (<a href="http://dx.doi.org/10.5220/0012084500003546" target="_blank" rel="noopener noreferrer">see paper</a>), and MB-EONs.  
- Support for customizable network topologies and multiple routing configurations via JSON files.  
- Support for heterogeneous bitrate traffic through JSON configuration.  
- Fine-grained control over connection arrival/departure ratios and the number of simulated connections.  
- Capability to define and generate custom statistics.  
- ...and more!  

<div class="section_buttons">
|                        Read Next |
|---------------------------------:|
| [Installation](@ref installation) |
</div>