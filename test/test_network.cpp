// Let Catch provide main():
#define CATCH_CONFIG_MAIN

#include "../src/core/network.hpp"
#include <algorithm>
#include "catch.hpp"

TEST_CASE("Constructor (Network) with JSON file") {
  // Test basic constructor
  CHECK_NOTHROW(std::make_shared<Network>());

  // Test JSON constructor with valid file doesn't throw
  CHECK_NOTHROW(Network("../examples/example_networks/NSFNet.json"));
  
  // Create object for further testing
  Network net("../examples/example_networks/NSFNet.json");

  // Name
  CHECK(net.getName() == "NSFNet");

  // Adjacency matrix: adjacencyMatrix[src][dst] = link_id, -1 if no link
  // Extracted from NSFNet.json links array
  int adjacencyMatrix[14][14] = {
      { -1,  21,  23,  -1,  -1,  -1,  -1,  -1,  27,  -1,  -1,  -1,  -1,  -1},  // 0
      {  0,  -1,  22,  26,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 1
      {  2,   1,  -1,  -1,  31,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 2
      { -1,   5,  -1,  -1,  -1,   4,  -1,  -1,  -1,  30,  -1,  -1,  -1,  -1},  // 3
      { -1,  -1,  10,  -1,  -1,   8,  -1,  11,  -1,  -1,  33,  -1,  -1,  -1},  // 4
      { -1,  -1,  -1,  25,  29,  -1,  24,  -1,  -1,  -1,  -1,  -1,  -1,  -1},  // 5
      { -1,  -1,  -1,  -1,  -1,   3,  -1,  13,  28,  -1,  -1,  -1,  -1,  -1},  // 6
      { -1,  -1,  -1,  -1,  32,  -1,  34,  -1,  -1,  -1,  -1,  -1,  -1,  14},  // 7
      {  6,  -1,  -1,  -1,  -1,  -1,   7,  -1,  -1,  -1,  -1,  -1,  -1,  36},  // 8
      { -1,  -1,  -1,   9,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  18,  37,  -1},  // 9
      { -1,  -1,  -1,  -1,  12,  -1,  -1,  -1,  -1,  -1,  -1,  20,  17,  -1},  // 10
      { -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  39,  41,  -1,  -1,  40},  // 11
      { -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  16,  38,  -1,  -1,  -1},  // 12
      { -1,  -1,  -1,  -1,  -1,  -1,  -1,  35,  15,  -1,  -1,  19,  -1,  -1}}; // 13

  // Test connectivity: adjacencyMatrix is indexed as [src][dst]
  for (int src = 0; src < 14; src++) {
    for (int dst = 0; dst < 14; dst++) {
      auto linkIds = net.isConnected(src, dst);
      if (adjacencyMatrix[src][dst] == -1) {
        CHECK(linkIds.empty());
      } else {
        REQUIRE_FALSE(linkIds.empty());
        CHECK(std::find(linkIds.begin(), linkIds.end(), adjacencyMatrix[src][dst]) != linkIds.end());
      }
    }
  }

  // Test with a file that doesn't exist
  std::string nonExistentFile = "../test/this_file_does_not_exist_12345.json";
  CHECK_THROWS_AS(Network(nonExistentFile), std::invalid_argument);
}

TEST_CASE("Heterogeneous Fiber Network") {
  Network network("../examples/example_networks/5_node_example.json");

  // Verify basic network structure
  CHECK(network.getNumberOfNodes() == 5);
  CHECK(network.getNumberOfLinks() == 16);

  // Test Node properties (only one direction per node pair)
  std::unique_ptr<Node>& nodeA = network.getNode("Node_A");
  CHECK(nodeA->getId() == 0);
  CHECK(nodeA->getDCs().value_or(-1) == 13);
  CHECK(nodeA->getIXPs().value_or(-1) == 3);
  CHECK(nodeA->getParam1().value_or(-1.0) == 10.5);
  CHECK(nodeA->getParam2().value_or(-1.0) == 20.3);

  // Link 0: SSMF with simple slot count
  auto link0 = network.getLink(0);
  CHECK(link0->getNumberOfFibers() == 1);
  auto fiber0 = link0->getFiber(0);
  CHECK(fiber0->getType() == fns::FiberType::SSMF);
  CHECK(fiber0->getNumberOfCores() == 1);
  CHECK(fiber0->getNumberOfModes(0, fns::Band::C) == 1);
  CHECK(fiber0->getNumberOfBands() == 1);
  CHECK(fiber0->getNumberOfSlots(0, fns::Band::C, 0) == 400);

  // Link 2: MCF with [400, 400, 400]
  auto link2 = network.getLink(2);
  CHECK(link2->getNumberOfFibers() == 1);
  auto fiber2 = link2->getFiber(0);
  CHECK(fiber2->getType() == fns::FiberType::MCF);
  CHECK(fiber2->getNumberOfCores() == 3);
  CHECK(fiber2->getNumberOfModes(0, fns::Band::C) == 1);
  CHECK(fiber2->getNumberOfBands() == 1);
  CHECK(fiber2->getNumberOfSlots(0, fns::Band::C, 0) == 400);
  CHECK(fiber2->getNumberOfSlots(1, fns::Band::C, 0) == 400);
  CHECK(fiber2->getNumberOfSlots(2, fns::Band::C, 0) == 400);

  // Link 4: FMF with [[400, 250, 250]]
  auto link4 = network.getLink(4);
  CHECK(link4->getNumberOfFibers() == 1);
  auto fiber4 = link4->getFiber(0);
  CHECK(fiber4->getType() == fns::FiberType::FMF);
  CHECK(fiber4->getNumberOfCores() == 1);
  CHECK(fiber4->getNumberOfModes(0, fns::Band::C) == 3);
  CHECK(fiber4->getNumberOfBands() == 1);
  CHECK(fiber4->getNumberOfSlots(0, fns::Band::C, 0) == 400);
  CHECK(fiber4->getNumberOfSlots(0, fns::Band::C, 1) == 250);
  CHECK(fiber4->getNumberOfSlots(0, fns::Band::C, 2) == 250);

  // Link 8: Multi-Band SSMF {"C": 400, "L": 520, "S": 1080}
  auto link8 = network.getLink(8);
  CHECK(link8->getNumberOfFibers() == 1);
  auto fiber8 = link8->getFiber(0);
  CHECK(fiber8->getType() == fns::FiberType::SSMF);
  CHECK(fiber8->getNumberOfCores() == 1);
  CHECK(fiber8->getNumberOfModes(0, fns::Band::C) == 1);
  CHECK(fiber8->getNumberOfBands() == 3);
  CHECK(fiber8->getNumberOfSlots(0, fns::Band::C, 0) == 400);
  CHECK(fiber8->getNumberOfSlots(0, fns::Band::L, 0) == 520);
  CHECK(fiber8->getNumberOfSlots(0, fns::Band::S, 0) == 1080);

  // Link 10: Multi-Band MCF {"C": [400, 250, 250], "L": [520, 250, 250]}
  auto link10 = network.getLink(10);
  CHECK(link10->getNumberOfFibers() == 1);
  auto fiber10 = link10->getFiber(0);
  CHECK(fiber10->getType() == fns::FiberType::MCF);
  CHECK(fiber10->getNumberOfCores() == 3);
  CHECK(fiber10->getNumberOfModes(0, fns::Band::C) == 1);
  CHECK(fiber10->getNumberOfBands() == 2);
  CHECK(fiber10->getNumberOfSlots(0, fns::Band::C, 0) == 400);
  CHECK(fiber10->getNumberOfSlots(1, fns::Band::C, 0) == 250);
  CHECK(fiber10->getNumberOfSlots(2, fns::Band::C, 0) == 250);
  CHECK(fiber10->getNumberOfSlots(0, fns::Band::L, 0) == 520);
  CHECK(fiber10->getNumberOfSlots(1, fns::Band::L, 0) == 250);
  CHECK(fiber10->getNumberOfSlots(2, fns::Band::L, 0) == 250);

  // Link 12: Complex Multi-Band FMMCF
  auto link12 = network.getLink(12);
  CHECK(link12->getNumberOfFibers() == 1);
  auto fiber12 = link12->getFiber(0);
  CHECK(fiber12->getType() == fns::FiberType::FMMCF);
  CHECK(fiber12->getNumberOfCores() == 2);
  CHECK(fiber12->getNumberOfModes(0, fns::Band::C) == 3);
  CHECK(fiber12->getNumberOfBands() == 3);
  CHECK(fiber12->getNumberOfSlots(0, fns::Band::C, 0) == 400);
  CHECK(fiber12->getNumberOfSlots(0, fns::Band::C, 1) == 250);
  CHECK(fiber12->getNumberOfSlots(0, fns::Band::C, 2) == 250);
  CHECK(fiber12->getNumberOfSlots(1, fns::Band::C, 0) == 400);
  CHECK(fiber12->getNumberOfSlots(1, fns::Band::C, 1) == 250);
  CHECK(fiber12->getNumberOfSlots(1, fns::Band::C, 2) == 250);
  CHECK(fiber12->getNumberOfSlots(0, fns::Band::L, 0) == 520);
  CHECK(fiber12->getNumberOfSlots(0, fns::Band::L, 1) == 250);
  CHECK(fiber12->getNumberOfSlots(0, fns::Band::L, 2) == 250);
  CHECK(fiber12->getNumberOfSlots(1, fns::Band::L, 0) == 520);
  CHECK(fiber12->getNumberOfSlots(1, fns::Band::L, 1) == 250);
  CHECK(fiber12->getNumberOfSlots(1, fns::Band::L, 2) == 250);
  CHECK(fiber12->getNumberOfSlots(0, fns::Band::S, 0) == 1080);
  CHECK(fiber12->getNumberOfSlots(0, fns::Band::S, 1) == 500);
  CHECK(fiber12->getNumberOfSlots(0, fns::Band::S, 2) == 500);
  CHECK(fiber12->getNumberOfSlots(1, fns::Band::S, 0) == 1080);
  CHECK(fiber12->getNumberOfSlots(1, fns::Band::S, 1) == 500);
  CHECK(fiber12->getNumberOfSlots(1, fns::Band::S, 2) == 500);

  // Link 14: Multi-fiber heterogeneous link
  auto link14 = network.getLink(14);
  CHECK(link14->getNumberOfFibers() == 5);
  // Fiber 0: SSMF
  auto fiber14_0 = link14->getFiber(0);
  CHECK(fiber14_0->getType() == fns::FiberType::SSMF);
  CHECK(fiber14_0->getNumberOfSlots(0, fns::Band::C, 0) == 400);
  // Fiber 1: MCF
  auto fiber14_1 = link14->getFiber(1);
  CHECK(fiber14_1->getType() == fns::FiberType::MCF);
  CHECK(fiber14_1->getNumberOfCores() == 4);
  CHECK(fiber14_1->getNumberOfSlots(0, fns::Band::C, 0) == 400);
  // Fiber 2: FMF
  auto fiber14_2 = link14->getFiber(2);
  CHECK(fiber14_2->getType() == fns::FiberType::FMF);
  CHECK(fiber14_2->getNumberOfModes(0, fns::Band::C) == 3);
  CHECK(fiber14_2->getNumberOfSlots(0, fns::Band::C, 0) == 400);
  // Fiber 3: Multi-Band SSMF
  auto fiber14_3 = link14->getFiber(3);
  CHECK(fiber14_3->getType() == fns::FiberType::SSMF);
  CHECK(fiber14_3->getNumberOfBands() == 4);
  CHECK(fiber14_3->getNumberOfSlots(0, fns::Band::E, 0) == 1120);
  // Fiber 4: Multi-Band FMMCF
  auto fiber14_4 = link14->getFiber(4);
  CHECK(fiber14_4->getType() == fns::FiberType::FMMCF);
  CHECK(fiber14_4->getNumberOfSlots(0, fns::Band::C, 0) == 400);
  CHECK(fiber14_4->getNumberOfSlots(1, fns::Band::C, 0) == 400);
};

TEST_CASE("Test Warning Example") {
  // Capture stderr to test the warning message
  std::ostringstream captured_cerr;
  std::streambuf* orig_cerr = std::cerr.rdbuf();
  std::cerr.rdbuf(captured_cerr.rdbuf());
  
  // Create network that should trigger the warning
  Network network("../test/test_warning_example.json");
  
  // Restore stderr
  std::cerr.rdbuf(orig_cerr);
  
  // Check that the warning was printed
  std::string warning_output = captured_cerr.str();
  CHECK(warning_output.find("Warning: Link") != std::string::npos);
  CHECK(warning_output.find("has link-level 'type' field that will be ignored") != std::string::npos);
  CHECK(warning_output.find("When using 'fibers' array, type must be set individually for each fiber") != std::string::npos);
};

TEST_CASE("Test Error Example") {
  // Test that loading a non bidirectional link configuration throws an error
  CHECK_THROWS_AS(Network("../test/test_error_links.json"), std::runtime_error);
};

TEST_CASE("Network Node Management") {
  Network network;
  
  // Test adding nodes
  auto node1 = std::make_unique<Node>(0, 1, 2, std::nullopt, "Node_0", std::nullopt, std::nullopt, 10.5, 20.3);
  auto node2 = std::make_unique<Node>(1, 3, 4, std::nullopt, "Node_1", std::nullopt, std::nullopt, 15.2, 25.8);
  
  network.addNode(std::move(node1));
  network.addNode(std::move(node2));
  
  // Test node count
  CHECK(network.getNumberOfNodes() == 2);
  
  // Test node retrieval by ID
  std::unique_ptr<Node>& retrievedNode = network.getNode(0);
  CHECK(retrievedNode->getId() == 0);
  CHECK(retrievedNode->getLabel().value_or("") == "Node_0");
  CHECK(retrievedNode->getDCs().value_or(-1) == 1);
  CHECK(retrievedNode->getIXPs().value_or(-1) == 2);
  
  // Test node retrieval by label
  std::unique_ptr<Node>& nodeByLabel = network.getNode("Node_1");
  CHECK(nodeByLabel->getId() == 1);
  CHECK(nodeByLabel->getDCs().value_or(-1) == 3);
  CHECK(nodeByLabel->getIXPs().value_or(-1) == 4);
  
  // Test out of bounds access
  CHECK_THROWS_AS(network.getNode(5), std::invalid_argument);
  
  // Test nonexistent label
  CHECK_THROWS_AS(network.getNode("NonExistent"), std::invalid_argument);
  
  // Test all nodes retrieval
  std::vector<const Node*> allNodes = network.getNodes();
  CHECK(allNodes.size() == 2);
  CHECK(allNodes[0]->getId() == 0);
  CHECK(allNodes[1]->getId() == 1);
}

TEST_CASE("Network Link Management") {
  Network network;
  
  // Add nodes first
  auto node1 = std::make_unique<Node>(0, 1, 1, std::nullopt, "Source", std::nullopt, std::nullopt, 0.0, 0.0);
  auto node2 = std::make_unique<Node>(1, 1, 1, std::nullopt, "Destination", std::nullopt, std::nullopt, 0.0, 0.0);
  network.addNode(std::move(node1));
  network.addNode(std::move(node2));
  
  // Verify nodes were added
  CHECK(network.getNumberOfNodes() == 2);
  
  // Create and add link
  auto link = std::make_shared<Link>(0, 100.0);
  auto fiber = std::make_shared<Fiber>(320); // Creates SSMF fiber with 320 slots in C band
  link->addFiber(fiber);
  network.addLink(link);
  
  // Verify link was added
  CHECK(network.getNumberOfLinks() == 1);
  
  // Before connection, src/dst should be -1
  std::shared_ptr<Link> linkBeforeConnect = network.getLink(0);
  CHECK(linkBeforeConnect->getSrc() == -1);
  CHECK(linkBeforeConnect->getDst() == -1);
  
  // Connect the nodes through the link (this sets src/dst on the link)
  network.connect(0, 0, 1);
  
  // After connection, check if src/dst are set correctly
  std::shared_ptr<Link> retrievedLink = network.getLink(0);
  CHECK(retrievedLink->getId() == 0);
  CHECK(retrievedLink->getLength() == 100.0);
  CHECK(retrievedLink->getSrc() == 0);
  CHECK(retrievedLink->getDst() == 1);
  
  // Test link retrieval by src/dst
  std::shared_ptr<Link> linkBySrcDst = network.getLink(0, 1);
  CHECK(linkBySrcDst->getId() == 0);
  
  // Test all links retrieval
  std::vector<std::shared_ptr<Link>> allLinks = network.getLinks();
  CHECK(allLinks.size() == 1);
  CHECK(allLinks[0]->getId() == 0);
  
  // Test nonexistent link
  CHECK_THROWS_AS(network.getLink(99), std::runtime_error);
  CHECK(network.getLink(5, 6) == nullptr); // This one returns nullptr for non-connected nodes
}

TEST_CASE("Network Connectivity Operations") {
  Network network;
  
  // Add nodes
  for (int i = 0; i < 3; i++) {
    auto node = std::make_unique<Node>(i, 1, 1, std::nullopt, "Node_" + std::to_string(i), std::nullopt, std::nullopt, 0.0, 0.0);
    network.addNode(std::move(node));
  }
  
  // Add links
  for (int i = 0; i < 2; i++) {
    auto link = std::make_shared<Link>(i, 100.0);
    auto fiber = std::make_shared<Fiber>(320); // Creates SSMF fiber with 320 slots in C band
    link->addFiber(fiber);
    network.addLink(link);
  }
  
  // Test connections
  network.connect(0, 0, 1);  // Connect node 0 to node 1 via link 0
  network.connect(1, 1, 2);  // Connect node 1 to node 2 via link 1
  
  // Test connectivity checks
  auto forwardLinks = network.isConnected(0, 1);
  REQUIRE_FALSE(forwardLinks.empty());
  CHECK(std::find(forwardLinks.begin(), forwardLinks.end(), 0) != forwardLinks.end());

  auto chainLinks = network.isConnected(1, 2);
  REQUIRE_FALSE(chainLinks.empty());
  CHECK(std::find(chainLinks.begin(), chainLinks.end(), 1) != chainLinks.end());

  auto missingLinks = network.isConnected(0, 2);
  CHECK(missingLinks.empty()); // Not directly connected
  
  // Test invalid connections
  CHECK_THROWS_AS(network.connect(0, 99, 1), std::runtime_error); // Invalid link
  CHECK_THROWS_AS(network.connect(99, 0, 1), std::runtime_error); // Invalid source node
  CHECK_THROWS_AS(network.connect(0, 0, 99), std::runtime_error); // Invalid destination node
}

TEST_CASE("Network Slot Management") {
  Network network;
  
  // Add nodes
  auto node1 = std::make_unique<Node>(0, 1, 1, std::nullopt, "A", std::nullopt, std::nullopt, 0.0, 0.0);
  auto node2 = std::make_unique<Node>(1, 1, 1, std::nullopt, "B", std::nullopt, std::nullopt, 0.0, 0.0);
  network.addNode(std::move(node1));
  network.addNode(std::move(node2));
  
  // Add link with fiber
  auto link = std::make_shared<Link>(0, 100.0);
  auto fiber = std::make_shared<Fiber>(320); // Creates SSMF fiber with 320 slots in C band
  link->addFiber(fiber);
  network.addLink(link);
  
  // Connect the nodes through the link (this sets src/dst on the link)
  network.connect(0, 0, 1);
  
  // Test slot usage
  int connectionId = 12345;
  
  // Use slots 10-14 on the fiber (slotTo is exclusive)
  CHECK_NOTHROW(network.useSlots(0, 0, 0, fns::Band::C, 0, 10, 15, connectionId));
  
  // Verify slots are used (10, 11, 12, 13, 14)
  auto retrievedLink = network.getLink(0);
  auto retrievedFiber = retrievedLink->getFiber(0);
  for (int slot = 10; slot < 15; slot++) {
    CHECK(retrievedFiber->getSlot(0, fns::Band::C, 0, slot) == connectionId);
  }
  
  // Test unused slots
  CHECK(retrievedFiber->getSlot(0, fns::Band::C, 0, 9) == 0);   // Before used range
  CHECK(retrievedFiber->getSlot(0, fns::Band::C, 0, 15) == 0);  // After used range (slotTo is exclusive)
  
  // Free the slots
  CHECK_NOTHROW(network.unuseSlots(0, 0, 0, fns::Band::C, 0, 10, 15));
  
  // Verify slots are free
  for (int slot = 10; slot < 15; slot++) {
    CHECK(retrievedFiber->getSlot(0, fns::Band::C, 0, slot) == 0);
  }
  
  // Test invalid slot operations
  CHECK_THROWS_AS(network.useSlots(99, 0, 0, fns::Band::C, 0, 10, 15, connectionId), std::out_of_range); // Invalid link
  CHECK_THROWS_AS(network.useSlots(0, 99, 0, fns::Band::C, 0, 10, 15, connectionId), std::out_of_range); // Invalid fiber
}

TEST_CASE("Network Empty State") {
  Network network;
  
  // Test empty network properties
  CHECK(network.getNumberOfNodes() == 0);
  CHECK(network.getNumberOfLinks() == 0);
  CHECK(network.getNodes().empty());
  CHECK(network.getLinks().empty());
  
  // Test accessing non-existent elements
  CHECK_THROWS_AS(network.getNode(0), std::invalid_argument);
  CHECK_THROWS_AS(network.getNode("NonExistent"), std::invalid_argument);
  CHECK_THROWS_AS(network.getLink(0), std::runtime_error);
  CHECK(network.getLink(0, 1) == nullptr);
  CHECK(network.isConnected(0, 1).empty());

  // Name should be "Unnamed Network" by default
  CHECK(network.getName() == "Unnamed Network");
}

TEST_CASE("JSON Export - networkToJson") {
  // Load the NSFNet network
  Network network("../examples/example_networks/NSFNet.json");
  
  // Export to JSON
  CHECK_NOTHROW(network.networkToJson());
  
  // Verify the file was created
  std::ifstream exportedFile("network_export.json");
  REQUIRE(exportedFile.is_open());
  
  // Parse the exported JSON
  nlohmann::json exported;
  exportedFile >> exported;
  exportedFile.close();
  
  // Verify network name
  CHECK(exported["name"] == "NSFNet");
  
  // Verify nodes
  REQUIRE(exported.contains("nodes"));
  REQUIRE(exported["nodes"].is_array());
  CHECK(exported["nodes"].size() == 14);
  
  // Check first node (Seattle)
  auto node0 = exported["nodes"][0];
  CHECK(node0["id"] == 0);
  CHECK(node0.contains("longitude"));
  CHECK(node0.contains("latitude"));
  CHECK(node0.contains("pop"));
  CHECK(node0.contains("DC"));
  CHECK(node0.contains("IXP"));
  
  // Verify links
  REQUIRE(exported.contains("links"));
  REQUIRE(exported["links"].is_array());
  CHECK(exported["links"].size() == 42);  // NSFNet has 42 links (21 bidirectional)
  
  // Check first link
  auto link0 = exported["links"][0];
  CHECK(link0["id"] == 0);
  CHECK(link0.contains("src"));
  CHECK(link0.contains("dst"));
  CHECK(link0.contains("length"));
  CHECK(link0.contains("slots"));
  
  // Reload the exported network to verify it's valid
  CHECK_NOTHROW(Network("network_export.json"));
  Network reloaded("network_export.json");
  
  // Verify the reloaded network has the same structure
  CHECK(reloaded.getName() == "NSFNet");
  CHECK(reloaded.getNumberOfNodes() == 14);
  CHECK(reloaded.getNumberOfLinks() == 42);
  
  // Clean up
  std::remove("network_export.json");
}

TEST_CASE("JSON Export - routesToJson") {
  // Load the NSFNet network
  Network network("../examples/example_networks/NSFNet.json");
  
  // Compute k-shortest paths (k=3)
  CHECK_NOTHROW(network.setPaths(3));
  
  // Export routes to JSON
  CHECK_NOTHROW(network.routesToJson());
  
  // Verify the file was created
  std::ifstream exportedFile("routes_export.json");
  REQUIRE(exportedFile.is_open());
  
  // Parse the exported JSON
  nlohmann::json exported;
  exportedFile >> exported;
  exportedFile.close();
  
  // Verify routes structure
  REQUIRE(exported.contains("routes"));
  REQUIRE(exported["routes"].is_array());
  CHECK(exported["routes"].size() > 0);
  
  // Check a route entry
  auto route0 = exported["routes"][0];
  CHECK(route0.contains("src"));
  CHECK(route0.contains("dst"));
  CHECK(route0.contains("paths"));
  REQUIRE(route0["paths"].is_array());
  CHECK(route0["paths"].size() <= 3);  // k=3 maximum
  
  // Verify each path is an array of link IDs
  if (route0["paths"].size() > 0) {
    auto path0 = route0["paths"][0];
    REQUIRE(path0.is_array());
    CHECK(path0.size() > 0);  // Should have at least one link
    // Each element should be a link ID (integer)
    for (const auto& linkId : path0) {
      CHECK(linkId.is_number_integer());
    }
  }
  
  // Test error when no paths are computed
  Network emptyNetwork;
  CHECK_THROWS_AS(emptyNetwork.routesToJson(), std::runtime_error);
  
  // Clean up
  std::remove("routes_export.json");
}
