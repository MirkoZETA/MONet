# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.0] - 2025-XX-XX

> **Note**: This first entry documents features and changes compared to the original [Flex Net Sim](https://gitlab.com/DaniloBorquez/flex-net-sim) project, which inspired MONet Library's architecture.

### CHANGED
- Extensive changes to original Flex Net Sim macros
- Event model changed from connection arrival/departure events to period-based updates with allocation executed once per period
- **Link class**:
  - Single-fiber and multi-fiber link configurations
  - Fiber bundle operations: add individual fibers or bundles of identical fiber types
  - Cable creation with predefined fiber type bundles (SSMF, MCF, FMF, FMMCF)
  - Per-fiber resource tracking and slot allocation
  - Link-level usage metrics aggregated across all fibers
- **Network class**:
  - JSON-based network definition with node attributes: ID, label, data centers (DCs), Internet Exchange Points (IXPs), population, coordinates (lat/lon), custom parameters (param1, param2)
  - Link attributes: ID, source, destination, length, fiber bundle
  - Network metrics: average neighborhood, nodal variance, link usage percentage
  - Network is now the owner of paths and is in charge of computing and reading route files
  - JSON export methods: `networkToJson()`, `routesToJson()` with 4-space indentation and ordered fields
- **Simulator class**:
  - Traffic model and growth model from *"Planning Optical Networks for Unexpected Traffic Growth"* Patri et al., (ECOC 2020) through `initializeDemands()` method
  - Normal distribution growth factors with configurable base rate and standard deviation
  - User-definable number of periods for long-term planning
  - ANSI color-coded console output: library name and completion time in bold cyan; underprovisioning percentage color-coded by severity (green for 0.0%, yellow 0.1-20%, orange 20-50%, bright red 50-80%, dark red >80%); bold attribute labels with italic network and algorithm names
  - Period-by-period demand tracking and provisioning
  - Event-driven simulation architecture with callback support
  - Dynamic topology modification (add nodes/links during simulation)
- **Connection class**:
  - P2P allocation flag (point-to-point dedicated fiber)
  - Dynamic bitrate modification for P2P connections
  - Counter-based ID management independent of vector indices
- **BitRate class**:
  - Multiple modulation format objects per bitrate (distance-adaptive schemes)
  - Distance-adaptive modulation selection: (1) reach requirement, (2) minimum slots (spectral efficiency), (3) maximum reach (reliability)
- **Controller class**:
  - JSON-based route file support for pre-computed paths
  - Compute K-shortest paths between node pairs using Yen's Algorithm with link lengths as weights
  - Path clearing and recomputation on topology changes
  - Automatic node degree calculation during adjacency list construction
  - JSON export method: `demandsToJson()`
- **Allocator**:
  - Paths are not shared pointers anymore, only owned by network class
- **JSON files**:
  - Routes are now using the link ID instead of nodes

### ADDED
- **Fiber class**: Multi-dimensional optical fiber representation
  - **Fiber types**: SSMF, FMF, MCF, FMMCF, HCF with automatic type detection
  - **Multi-band support**: C, L, S, E, U, O bands with independent configurations
  - **Multi-core architecture**: Variable core counts per fiber (1 to N cores)
  - **Multi-mode transmission**: Variable mode counts per core (1 to M modes per core)
  - **Heterogeneous configurations**: Each core can have different mode counts; each core-mode can have different slot counts
  - **Spectral resource management**: Per-band, per-core, per-mode slot allocation tracking
  - **Dynamic band addition**: Add new spectral bands to existing fibers at runtime
  - **Flexible slot configuration**: Individual slot count specification for each [band][core][mode] combination
- **Demand class**: Traffic requirements between node pairs
  - Required capacity tracking (driven by growth models)
  - Allocated capacity tracking (actual provisioned capacity)
  - Unprovisioned capacity calculation (gap analysis)
  - Capacity addition/subtraction for incremental provisioning
  - Provisioning status tracking (fully provisioned vs. under-provisioned)
  - Per-demand ID, source, destination attributes
- **P2P class**: Dedicated fiber paths between node pairs
  - Direct node-to-node connections using dedicated fibers
  - Fiber sharing with regular links (some fibers for P2P, others for routing)
  - Multi-fiber P2P paths with per-fiber resource tracking
  - Bulk slot reservation across all fibers in P2P container
  - Connection migration from routed to P2P paths
- **ModulationFormat class**: Optical modulation scheme representation
  - Modulation name (e.g., BPSK, QPSK, 16-QAM, 32-QAM, 64-QAM)
  - Band-specific required slots (different spectral efficiency per band)
  - Band-specific maximum reach distances
  - GSNR requirements (reserved for future physical layer integration)

[0.1.0]: https://gitlab.com/DaniloBorquez/flex-net-sim/-/releases/v0.1.0
