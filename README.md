# MONet Library
## Multi-period Optical Network Evaluation Tool

[![Static Badge](https://img.shields.io/badge/version-0.1.0-blue)](https://gitlab.com/DaniloBorquez/flex-net-sim)
![Static Badge](https://img.shields.io/badge/language-C%2B%2B-blue)
[![Static Badge](https://img.shields.io/badge/licese-MIT-green)](https://gitlab.com/DaniloBorquez/flex-net-sim/-/blob/master/LICENSE.md?ref_type=heads)

# MONet Library

**MONet** (Multi-period Optical Network Evaluation Tool) is a C++ library for multi-period core optical network planning and evaluation. It enables researchers and engineers to implement and test custom allocation strategies (heuristics, ILPs, etc.) under realistic traffic growth scenarios.

The library implements the traffic generation and growth model described in Patri et al., *"Planning Optical Networks for Unexpected Traffic Growth"* (ECOC 2020), which models realistic traffic evolution across multiple planning periods.

## Key Features

- **Multi-period planning**: Simulate network evolution over multiple time periods with configurable traffic growth models.
- **Static/Incremental traffic**: Traffic demands grow incrementally between periods events.
- **Flexible network architectures**: Support for Elastic Optical Networks (EONs), Multi-Band (MB), Spatial Division Multiplexing (SDM), and Multi-Fiber (MF) links with any combination of these technologies.
- **Heterogeneous networks**: Model realistic networks with heterogeneous fibers, where different links can have different fiber types, bands, cores, modes, and slot configurations.
- **Dynamic network topology**: Add nodes and links on-the-fly during simulation to model network expansion.
- **User-defined allocation algorithms**: Implement custom resource allocation strategies using a simple macro-based API.
- **Customizable topologies**: Define network topologies via JSON files or use built-in examples.
- **Flexible routing**: Provide pre-calculated k-shortest paths or let the library compute them automatically.
- **Heterogeneous bitrates**: Configure multiple bitrate services with band-specific modulation formats via JSON.
- **Demand modeling**: Automatic demand generation based on network topology or user-provided demand matrices.

## Getting Started

### Requirements

**Linux (Ubuntu/Debian)**

- **C++ Compiler**: GCC 7+ or Clang 5+ (C++17 support required)
- **CMake**: Version 3.10 or higher
- **Git**: For cloning the repository

Install on Ubuntu/Debian:
```bash
sudo apt update
sudo apt install build-essential cmake git
```

### Installation

1. **Clone the repository**:
```bash
git clone https://gitlab.com/DaniloBorquez/flex-net-sim.git
cd flex-net-sim
```

2. **Build the library**:
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

3. **Run tests** (optional):
```bash
# Run all tests
./test/test_bitrate && ./test/test_connection && ./test/test_controller && \
./test/test_demand && ./test/test_fiber && ./test/test_link && \
./test/test_modulation_format && ./test/test_network && ./test/test_node && \
./test/test_random_variable && ./test/test_simulator
```

### Basic Example

TODO

## Coming Soon

The following features are currently under development:

- **Physical layer integration**: GNPy integration for realistic physical layer impairment modeling in C and C+L band scenarios.
- **Custom traffic demand models**: Define traffic demands based on node attributes such as data centers (DCs), internet exchange points (IXPs), population, or up to two custom parameters (param1, param2).
- **Failure simulation**: Inter-period component failure simulation (links, nodes, fibers) with customizable failure management routines via macros.
- **Comprehensive documentation**: Detailed user guide and tutorials.

## Citation

If you use MONet in your research, please cite the original Flex Net Sim work:

```bibtex
@misc{borquez2021,
  author        = {Felipe Falcón and Gonzalo España and Danilo Bórquez-Paredes},
  title         = {Flex Net Sim: A Lightweight Manual},
  year          = {2021},
  eprint        = {2105.02762},
  archivePrefix = {arXiv},
  primaryClass  = {cs.NI}
}
```

## Acknowledgments

Special thanks to:
- [Danilo Bórquez-Paredes](https://gitlab.com/DaniloBorquez) - Original Flex Net Sim author
- [Christofer Vásquez Farías](https://gitlab.com/christofer.vasquez.farias) - MONet contributor
- The Flex Net Sim team for their foundational work

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details.