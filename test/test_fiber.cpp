#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "../src/core/fiber.hpp"

TEST_CASE("Constructor (Fiber)")
{
  // Valid constructors
  CHECK_NOTHROW(Fiber());

  // Only slots as parameter
  CHECK_NOTHROW(Fiber(fns::defaults::SLOTS));

  // All specified
  std::map<fns::Band,std::vector<std::vector<int>>> bandSlotMatrix = {
      {fns::Band::C, {{fns::defaults::SLOTS}, {400}}},
      {fns::Band::L, {{fns::defaults::SLOTS}, {400}}}
  };
  CHECK_NOTHROW(Fiber(bandSlotMatrix));

// Different mode count for core 0
  std::map<fns::Band, std::vector<std::vector<int>>> varyingModes = {
      {fns::Band::C, {{fns::defaults::SLOTS}, {400}}},
      {fns::Band::L, {{fns::defaults::SLOTS, 200}, {400}}} 
  };
  CHECK_NOTHROW(Fiber(varyingModes));

  // Invalid constructors
  CHECK_THROWS_AS(Fiber(0), std::invalid_argument);
  CHECK_THROWS_AS(Fiber(-1), std::invalid_argument);
  std::map<fns::Band, std::vector<std::vector<int>>> invalidSlots = {
      {fns::Band::C, {{fns::defaults::SLOTS}, {0}}}, // Invalid slots
      {fns::Band::L, {{fns::defaults::SLOTS}, {400}}}
  };
  CHECK_THROWS_AS(Fiber(invalidSlots), std::invalid_argument);
  std::map<fns::Band, std::vector<std::vector<int>>> invalidCores1 = {
      {fns::Band::C, {{fns::defaults::SLOTS}, {400}}},
      {fns::Band::L, {{fns::defaults::SLOTS}}} // Different core count
  };
  CHECK_THROWS_AS(Fiber(invalidCores1), std::invalid_argument);
  std::map<fns::Band, std::vector<std::vector<int>>> invalidCores2 = {
      {fns::Band::C, {{fns::defaults::SLOTS}, {400}}},
      {fns::Band::L, {{fns::defaults::SLOTS}, {400}, {300}}} // Different core count
  };
  CHECK_THROWS_AS(Fiber(invalidCores2), std::invalid_argument);
  std::map<fns::Band, std::vector<std::vector<int>>> invalidBands = {
      {fns::Band::C, {{fns::defaults::SLOTS}, {400}}},
      {fns::Band::L, {{fns::defaults::SLOTS}, {400}}},
      {fns::Band::S, {}} // Empty matrix
  };
  CHECK_THROWS_AS(Fiber(invalidBands), std::invalid_argument);
  std::map<fns::Band, std::vector<std::vector<int>>> emptyMatrix = {};
  CHECK_THROWS_AS(Fiber(emptyMatrix), std::invalid_argument);
}

TEST_CASE("Fiber Type Detection and Management")
{
  // Test default constructor - should be SSMF (1 core, 1 mode, 1 band)
  Fiber ssmf;
  CHECK(ssmf.getType() == fns::FiberType::SSMF);
  CHECK(ssmf.getNumberOfCores() == 1);
  CHECK(ssmf.getNumberOfModes(0, fns::Band::C) == 1);
  CHECK(ssmf.getNumberOfBands() == 1);
  
  // Test FMF (1 core, multiple modes)
  std::map<fns::Band, std::vector<std::vector<int>>> fmfMatrix = {
      {fns::Band::C, {{200, 320, 100}}} // 1 core, 3 modes
  };
  Fiber fmf(fmfMatrix);
  CHECK(fmf.getType() == fns::FiberType::FMF);
  CHECK(fmf.getNumberOfCores() == 1);
  CHECK(fmf.getNumberOfModes(0, fns::Band::C) == 3);
  CHECK(fmf.getNumberOfBands() == 1);
  CHECK(fmf.getNumberOfSlots(0, fns::Band::C, 2) == 100); // Check slot count for mode 2
  
  // Test MCF (multiple cores, 1 mode)
  std::map<fns::Band, std::vector<std::vector<int>>> mcfMatrix = {
      {fns::Band::C, {{100}, {200}, {150}, {250}}} // 4 cores, 1 mode each
  };
  Fiber mcf(mcfMatrix);
  CHECK(mcf.getType() == fns::FiberType::MCF);
  CHECK(mcf.getNumberOfCores() == 4);
  CHECK(mcf.getNumberOfModes(0, fns::Band::C) == 1);
  CHECK(mcf.getNumberOfModes(1, fns::Band::C) == 1);
  CHECK(mcf.getNumberOfModes(2, fns::Band::C) == 1);
  CHECK(mcf.getNumberOfModes(3, fns::Band::C) == 1);
  CHECK(mcf.getNumberOfBands() == 1);
  CHECK(mcf.getNumberOfSlots(2, fns::Band::C, 0) == 150); // Check slot count for core 2
  
  // Test FMMCF (multiple cores, multiple modes)
  std::map<fns::Band, std::vector<std::vector<int>>> fmmcfMatrix = {
      {fns::Band::C, {{100, 80}, {90, 70}, {120, 110}}}, // 3 cores, 2 modes each
      {fns::Band::L, {{80, 60}, {70, 50}, {130, 90}}}    // 3 cores, 2 modes each
  };
  Fiber fmmcf(fmmcfMatrix);
  CHECK(fmmcf.getType() == fns::FiberType::FMMCF);
  CHECK(fmmcf.getNumberOfCores() == 3);
  CHECK(fmmcf.getNumberOfModes(0, fns::Band::C) == 2);
  CHECK(fmmcf.getNumberOfModes(1, fns::Band::C) == 2);
  CHECK(fmmcf.getNumberOfModes(2, fns::Band::C) == 2);
  CHECK(fmmcf.getNumberOfBands() == 2);
  CHECK(fmmcf.getNumberOfSlots(0, fns::Band::C, 0) == 100); // Check total slot count for core 0, mode 0 in C band
  CHECK(fmmcf.getNumberOfSlots(1, fns::Band::L, 1) == 50); // Check slot count for core 1, mode 1 in L band
  CHECK(fmmcf.getNumberOfSlots(0, fns::Band::C, 1) == 80); // Check slot count for core 0, mode 1 in C band

  // Test manual type setting
  ssmf.setType(fns::FiberType::HCF);
  CHECK(ssmf.getType() == fns::FiberType::HCF);
}

TEST_CASE("Band Management")
{
  Fiber fiber;
  
  // Initially should have only C band
  CHECK(fiber.getNumberOfBands() == 1);
  auto bands = fiber.getBands();
  CHECK(bands.size() == 1);
  CHECK(bands[0] == fns::Band::C);
  
  // Add new bands
  CHECK_NOTHROW(fiber.addBand(fns::Band::L, 50, 1));
  CHECK_NOTHROW(fiber.addBand(fns::Band::S, 75, 1));
  CHECK(fiber.getNumberOfBands() == 3);

  CHECK_THROWS_AS(fiber.addBand(fns::Band::C, 100, 1), std::invalid_argument); // Duplicate band
  CHECK_THROWS_AS(fiber.addBand(fns::Band::E, 100, 0), std::invalid_argument); // Non-positive modes
  CHECK_THROWS_AS(fiber.addBand(fns::Band::E, 0, 1), std::invalid_argument); // Non-positive slot
  bands = fiber.getBands();
  CHECK(bands.size() == 3);

	std::vector<fns::Band> expected{ fns::Band::C, fns::Band::L, fns::Band::S };
	std::sort(bands.begin(), bands.end());
	std::sort(expected.begin(), expected.end());
	CHECK(bands == expected);
  
  CHECK_NOTHROW(fiber.clearFiber());
  CHECK_NOTHROW(fiber.addBand(fns::Band::C, 100, 1)); // Should not throw because fiber was cleared
  CHECK(fiber.getNumberOfBands() == 1); // Now should have 1
  CHECK(fiber.getNumberOfCores() == 1); // Ensure cores was created with 1
}


TEST_CASE("Core Management") 
{
  std::map<fns::Band, std::vector<std::vector<int>>> bandSlotMatrix = {
      {fns::Band::C, {{100}, {200}, {150}}}, // 3 cores, 1 mode each
  };
  Fiber fiber(bandSlotMatrix);

  // Initially should have 3 cores, 1 mode each
  CHECK(fiber.getNumberOfCores() == 3);
  CHECK(fiber.getNumberOfModes(0, fns::Band::C) == 1);

  // Test setting cores
  std::vector<std::vector<int>> newCoreConfig = {
      {100, 100}, // Core 0: 2 modes
      {200},      // Core 1: 1 mode
      {150, 150, 150} // Core 2: 3 modes
  };
  CHECK_NOTHROW(fiber.setCores(newCoreConfig));
  CHECK(fiber.getNumberOfCores() == 3);
  CHECK(fiber.getNumberOfModes(0, fns::Band::C) == 2);
  CHECK(fiber.getNumberOfModes(1, fns::Band::C) == 1);
  CHECK(fiber.getNumberOfModes(2, fns::Band::C) == 3);

  // Test invalid core configurations
  CHECK_THROWS_AS(fiber.setCores({}), std::invalid_argument); // Empty
  CHECK_THROWS_AS(fiber.setCores({{100}, {}}), std::invalid_argument); // Core with no modes
  CHECK_THROWS_AS(fiber.setCores({{100}, {0}}), std::invalid_argument); // Non-positive slots
  fiber.setSlot(0, fns::Band::C, 0, 0, 1); // Allocate a slot
  CHECK_THROWS_AS(fiber.setCores(newCoreConfig), std::runtime_error);
}

TEST_CASE("Mode Management")
{
  std::map<fns::Band, std::vector<std::vector<int>>> bandSlotMatrix = {
      {fns::Band::C, {{100, 200, 100, 100, 100, 100, 200}}}, // 1 core, 7 modes
  };
  Fiber fiber(bandSlotMatrix);

  // Initially should have 1 core, 7 modes
  CHECK(fiber.getNumberOfCores() == 1);
  CHECK(fiber.getNumberOfModes(0, fns::Band::C) == 7);

  // Test getting modes
  CHECK_THROWS_AS(fiber.getNumberOfModes(0, fns::Band::L), std::invalid_argument); // Band not found
  CHECK_THROWS_AS(fiber.getNumberOfModes(2, fns::Band::C), std::out_of_range);
    
  // Test setting modes
  std::vector<int> slotsPerMode = {100, 100, 100};
  CHECK_NOTHROW(fiber.setModes(0, fns::Band::C, slotsPerMode));
  CHECK(fiber.getNumberOfModes(0, fns::Band::C) == 3);
  CHECK_THROWS_AS(fiber.setModes(1, fns::Band::C, slotsPerMode), std::out_of_range); // Core out of range
  CHECK_THROWS_AS(fiber.setModes(0, fns::Band::L, slotsPerMode), std::invalid_argument); // Band not found
  CHECK_THROWS_AS(fiber.setModes(0, fns::Band::C, {}), std::invalid_argument); // Empty vector
  CHECK_THROWS_AS(fiber.setModes(0, fns::Band::C, {100, -50}), std::invalid_argument); // Invalid slot count
  fiber.setSlot(0, fns::Band::C, 0, 0, 1); // Allocate a slot
  CHECK_THROWS_AS(fiber.setModes(0, fns::Band::C, slotsPerMode), std::runtime_error); // Cannot change modes when slots allocated
}

TEST_CASE("Slot Management - Getting and Setting")
{
    // Create fiber with custom configuration
    std::map<fns::Band, std::vector<std::vector<int>>> config = {
        {fns::Band::C, {{100, 200}, {150}}}, // Core 0: 2 modes, Core 1: 1 mode
        {fns::Band::L, {{80, 120}, {100}}}   // Same structure for L band
    };
    Fiber fiber(config);
    
    // Test getter methods
    CHECK(fiber.getNumberOfSlots(0, fns::Band::C, 0) == 100); // First band, first core, first mode
    CHECK(fiber.getNumberOfSlots(0, fns::Band::C, 1) == 200);
    CHECK(fiber.getNumberOfSlots(1, fns::Band::C, 0) == 150);
    CHECK(fiber.getNumberOfSlots(0, fns::Band::L, 0) == 80);
    CHECK(fiber.getNumberOfSlots(0, fns::Band::L, 1) == 120);
    CHECK(fiber.getNumberOfSlots(1, fns::Band::L, 0) == 100);
    
    // Test setter methods - global
    CHECK_NOTHROW(fiber.setSlots(0, fns::Band::C, 0, 300));
    CHECK(fiber.getNumberOfSlots(0, fns::Band::C, 0) == 300);
    CHECK(fiber.getNumberOfSlots(0, fns::Band::C, 1) == 200);
    CHECK(fiber.getNumberOfSlots(1, fns::Band::C, 0) == 150);
    CHECK(fiber.getNumberOfSlots(0, fns::Band::L, 0) == 80);
    CHECK(fiber.getNumberOfSlots(0, fns::Band::L, 1) == 120);
    CHECK(fiber.getNumberOfSlots(1, fns::Band::L, 0) == 100);
    
    // Test setter methods - per core
    CHECK_NOTHROW(fiber.setSlots(1, fns::Band::C, 0, 250));
    CHECK(fiber.getNumberOfSlots(0, fns::Band::C, 0) == 300);
    CHECK(fiber.getNumberOfSlots(0, fns::Band::C, 1) == 200);
    CHECK(fiber.getNumberOfSlots(1, fns::Band::C, 0) == 250);
    CHECK(fiber.getNumberOfSlots(0, fns::Band::L, 0) == 80);
    CHECK(fiber.getNumberOfSlots(0, fns::Band::L, 1) == 120);
    CHECK(fiber.getNumberOfSlots(1, fns::Band::L, 0) == 100);
    
    // Test setter methods - specific core/mode
    CHECK_NOTHROW(fiber.setSlots(0, fns::Band::C, 1, 180));
    CHECK(fiber.getNumberOfSlots(0, fns::Band::C, 0) == 300);
    CHECK(fiber.getNumberOfSlots(0, fns::Band::C, 1) == 180);
    CHECK(fiber.getNumberOfSlots(1, fns::Band::C, 0) == 250);
    CHECK(fiber.getNumberOfSlots(0, fns::Band::L, 0) == 80);
    CHECK(fiber.getNumberOfSlots(0, fns::Band::L, 1) == 120);
    CHECK(fiber.getNumberOfSlots(1, fns::Band::L, 0) == 100);
    
    // Test setter methods - band specific
    CHECK_NOTHROW(fiber.setSlots(0, fns::Band::L, 0, 90));
    CHECK(fiber.getNumberOfSlots(0, fns::Band::C, 0) == 300);
    CHECK(fiber.getNumberOfSlots(0, fns::Band::C, 1) == 180);
    CHECK(fiber.getNumberOfSlots(1, fns::Band::C, 0) == 250);
    CHECK(fiber.getNumberOfSlots(0, fns::Band::L, 0) == 90);
    CHECK(fiber.getNumberOfSlots(0, fns::Band::L, 1) == 120);
    CHECK(fiber.getNumberOfSlots(1, fns::Band::L, 0) == 100);
    
    // Test invalid operations
    CHECK_THROWS_AS(fiber.setSlots(0, fns::Band::S, 0, 100), std::invalid_argument); // Band not found
    CHECK_THROWS_AS(fiber.setSlots(2, fns::Band::C, 0, 100), std::out_of_range); // Core out of range
    CHECK_THROWS_AS(fiber.setSlots(0, fns::Band::C, 2, 100), std::out_of_range); // Mode out of range
    CHECK_THROWS_AS(fiber.setSlots(0, fns::Band::C, 0, -50), std::invalid_argument); // Negative slots

    // Test range validation
    CHECK_THROWS_AS(fiber.getNumberOfSlots(0, fns::Band::E, 0), std::invalid_argument);
    CHECK_THROWS_AS(fiber.getNumberOfSlots(-1, fns::Band::C, 0), std::out_of_range);
    CHECK_THROWS_AS(fiber.getNumberOfSlots(0, fns::Band::C, -1), std::out_of_range); 
}

TEST_CASE("Slot Allocation and Fiber Status")
{
    Fiber fiber(150); // Simple fiber with 150 slots
    
    // Initially inactive
    CHECK_FALSE(fiber.isActive());
    
    // Test slot allocation
    CHECK_NOTHROW(fiber.setSlot(0, fns::Band::C, 0, 0, 1));
    CHECK(fiber.getSlot(0, fns::Band::C, 0, 0) == 1);
    CHECK(fiber.isActive()); // Now active
    
    CHECK_NOTHROW(fiber.setSlot(0, fns::Band::C, 0, 149, 2));
    CHECK(fiber.getSlot(0, fns::Band::C, 0, 149) == 2);
    
    // Test unallocated slots
    CHECK(fiber.getSlot(0, fns::Band::C, 0, 1) == -1);
    CHECK(fiber.getSlot(0, fns::Band::C, 0, 50) == -1);
    
    // Test dedicated P2P flag - first reset fiber to clear allocations
    fiber.resetFiber();
    CHECK_FALSE(fiber.isActive()); // Should be inactive after reset
    CHECK_FALSE(fiber.isDedicatedToP2P()); // Initially not dedicated
    CHECK_NOTHROW(fiber.setDedicatedToP2P(true));
    CHECK(fiber.isDedicatedToP2P());
    CHECK_NOTHROW(fiber.setDedicatedToP2P(false));
    CHECK_FALSE(fiber.isDedicatedToP2P());
    
    // Test error when trying to dedicate an active fiber
    fiber.setSlot(0, fns::Band::C, 0, 10, 5); // Make it active
    CHECK_THROWS_AS(fiber.setDedicatedToP2P(true), std::runtime_error);
    
    // Test validation errors
    CHECK_THROWS_AS(fiber.setSlot(-1, fns::Band::C, 0, 0, 1), std::out_of_range);
    CHECK_THROWS_AS(fiber.setSlot(0, fns::Band::L, 0, 0, 1), std::invalid_argument); // Band not found
    CHECK_THROWS_AS(fiber.setSlot(0, fns::Band::C, -1, 0, 1), std::out_of_range);
    CHECK_THROWS_AS(fiber.setSlot(0, fns::Band::C, 0, 150, 1), std::out_of_range); // Slot out of range
    CHECK_THROWS_AS(fiber.getSlot(0, fns::Band::C, 1, 0), std::out_of_range); // Mode out of range
}

TEST_CASE("Fiber Reset and Clear Operations")
{
    std::map<fns::Band, std::vector<std::vector<int>>> config = {
        {fns::Band::C, {{100}}},
        {fns::Band::L, {{80}}}
    };
    Fiber fiber(config);
    
    // Allocate some slots
    fiber.setSlot(0, fns::Band::C, 0, 0, 1);
    fiber.setSlot(0, fns::Band::L, 0, 10, 2);
    CHECK(fiber.isActive());
    
    // Test resetFiber - keeps structure, clears allocations
    CHECK_NOTHROW(fiber.resetFiber());
    CHECK_FALSE(fiber.isActive());
    CHECK(fiber.getNumberOfBands() == 2); // Structure preserved
    CHECK(fiber.getNumberOfSlots(0, fns::Band::C, 0) == 100);
    CHECK(fiber.getSlot(0, fns::Band::C, 0, 0) == -1); // Allocation cleared
    
    // Allocate again and test clearFiber
    fiber.setSlot(0, fns::Band::C, 0, 0, 3);
    CHECK(fiber.isActive());
    
    // clearFiber removes everything (with warning to stderr)
    CHECK_NOTHROW(fiber.clearFiber());
    CHECK_FALSE(fiber.isActive());
    CHECK(fiber.getNumberOfBands() == 0); // Structure removed
}

TEST_CASE("Static Utility Functions")
{
    // Test stringToFiberType
    CHECK(fns::stringToFiberType("SSMF") == fns::FiberType::SSMF);
    CHECK(fns::stringToFiberType("FMF") == fns::FiberType::FMF);
    CHECK(fns::stringToFiberType("MCF") == fns::FiberType::MCF);
    CHECK(fns::stringToFiberType("FMMCF") == fns::FiberType::FMMCF);
    CHECK(fns::stringToFiberType("HCF") == fns::FiberType::HCF);
    CHECK_THROWS_AS(fns::stringToFiberType("UNKNOWN"), std::runtime_error);
    CHECK_THROWS_AS(fns::stringToFiberType(""), std::runtime_error);
    
    // Test charToBand
    CHECK(fns::charToBand('C') == fns::Band::C);
    CHECK(fns::charToBand('L') == fns::Band::L);
    CHECK(fns::charToBand('S') == fns::Band::S);
    CHECK(fns::charToBand('E') == fns::Band::E);
    CHECK(fns::charToBand('U') == fns::Band::U);
    CHECK(fns::charToBand('O') == fns::Band::O);
    CHECK_THROWS_AS(fns::charToBand('X'), std::runtime_error);
    CHECK_THROWS_AS(fns::charToBand('1'), std::runtime_error);
}