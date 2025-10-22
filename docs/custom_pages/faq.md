@page faq Frequently Asked Questions

@tableofcontents

@section q1 How does the library handle physical impairments?
Flex Net Sim primarily focuses on network-level simulations and resource allocation. As a result, it accounts for physical layer impairments using predefined reach tables for each *modulation format–bit rate* pair, considering only the worst-case scenario.

These values can be easily set via a user-defined bit rate file.

The library is flexible enough to support online impairment calculations, but implementing them requires additional C++ programming expertise from the user.

@section q2 How can I adjust spectral granularity?

Since the library operates with slots, granularity is inherently abstracted within the simulation. Conceptually, each slot can represent different spectral widths, such as *50 GHz* or *6.25 GHz*, and the library will handle them uniformly as slot units.

To adjust granularity, you should modify the number of slots based on the available spectral bandwidth. For example, if a *12.5 GHz* allocation allows for 6 slots, then at *25 GHz*, only 3 slots would be available. The key is to scale the slot count according to spectral width while maintaining consistency in slot-based resource allocation.

@note
Ensure that the slot granularity considered in the network *JSON* file is consistent with the one in the bit rate *JSON* file.

@section q3 How does the library compute routes?
Among the [necessary files](@ref files) required to use the library, you must provide the network structure and its corresponding precomputed paths. This means the library **does not compute routes autonomously** (yet).

However, you can use [this auxiliary Colab script](https://colab.research.google.com/drive/1bqpKjn1MBhnm1H9ffX1fOSTlHHlAyB6H?usp=sharing) to generate your own network and route file using the **k-shortest path**. Alternatively, you can use our predefined [JSON network files](https://gitlab.com/DaniloBorquez/flex-net-sim/-/tree/master/networks?ref_type=heads).

We are continuously expanding the set of available networks, and in the near future, we plan to integrate a built-in route computation process.