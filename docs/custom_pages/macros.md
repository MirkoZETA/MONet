@page full-macros Full List of Macros

@todo
Complete, explain, etc.

- **BEGIN_ALLOC_FUNCTION(name__)** can be used to create an allocation algorithm that will belong to the allocator class. The parameter name__ corresponds to the name given to the new method, and so it can be used to call it when required. END_ALLOC_FUNCTION must be written after the algorithm to indicate the end of the function.
- **USE_ALLOC_FUNCTION(fun, simObject)** can be used on the main file to set the allocation algorithm to be used by the simulation. The name of said algorithm is passed as the 'fun' parameter, and simObject corresponds to the simulator object whose allocator will be set.
- **SRC** and **DST** correspond to the source and destination nodes of an event.
- **REQ_SLOTS_FIXED** retrieves the number of slots required for a connection assuming fixed-rate.
- **REQ_SLOTS(pos)** can be used to get the number of slots required for a connection between nodes. The pos paremeter refers to the required position in the slots vector of the bitRate object.
- **REQ_REACH_FIXED** retrieves the optical reach assuming fixed-rate (default position `0` in the reach vector).  
- **REQ_REACH(pos)** Get the optical reach located in the *pos* on the JSON file 
- **REQ_REACH(pos)** Get the optical reach located in the *pos* on the JSON file 
- **REQ_MODULATION(pos)** Get the modulation format located in the *pos* on the JSON file
- **REQ_BITRATE_STR** Get the bitrate value as string
- **REQ_BITRATE Get** the bitrate value as double
- **LINK_IN_ROUTE(route, link)** can be used to access a specific link object on a route between a source and destination node.
- **LINK_IN_ROUTE_ID(route, link)** can be used to access the id of a specific link on a route between a source and destination node.
- **NUMBER_OF_ROUTES** represents the amount of routes that exist between the current source and destination node.
- **NUMBER_OF_LINKS(route)** represents the amount of links between a source and destination node on the specified route.
- **NUMBER_OF_CORES(route, linkIndex)** represents the amount of cores between a source and destination node on the specified route and link index.
- **NUMBER_OF_MODES(route, linkIndex, core)** represents the amount of modes .between a source and destination node on the specified route and link index.
- **VECTOR_OF_BANDS(route, linkIndex)** Get a vector of char values representing the bands from the link.
- **NUMBER_OF_BANDS(route, linkIndex)** Get the Number of bands attribute of the Link object.
- **REQ_POS_BANDS(pos)** Retrieves the positions of the bands within the modulation.
- **REQ_SLOTS_BDM(pos, bandIndex)** can be used to get the number of slots required for a connection between nodes. The 'pos' parameter pertains to the necessary position within the slots vector of the 'bitRate' object, while the 'bandIndex' parameter pertains to the required position of the band.
- **REQ_REACH_BDM(pos, bandIndex)** Get the optical reach located at pos in the JSON file along with the band index.
- **ALLOC_SLOTS(link, from, to)** can be used to create a connection on the required interval of slots *[from, from+to[* of the specified link.
- **ALLOC_SLOTS_SDM(link, core, mode, from, to)** can be used to create a connection on the required interval of slots *[from, from+to[* of the specified link, core and mode.
- **ALLOC_SLOTS_BDM(link, band, from, to)** can be used to create a connection on the required interval of slots *[from, from+to[* of the specified link and band.
- **USE_UNALLOC_FUNCTION_SDM(fun, simObject)** can be used on the main file to set the callback unallocation algorithm to be used by the simulation. The name of said algorithm is passed as the 'fun' parameter, and simObject corresponds to the simulator object.
- **USE_UNALLOC_FUNCTION_BDM(simObject)** can be used on the main file to set the callback unallocation algorithm to be used by the simulation. 
- **BEGIN_UNALLOC_CALLBACK_FUNCTION** can be used to create a callback unallocation algorithm. This function is triggered immediately a departure occurs. END_UNALLOC_CALLBACK_FUNCTION must be written after the algorithm to indicate the end of the function.