// Let Catch provide main():
#define CATCH_CONFIG_MAIN

#include "../src/core/link.hpp"
#include "../src/core/fiber.hpp"
#include "catch.hpp"

TEST_CASE("Constructor (Link)")
{
    // Valid constructors
    CHECK_NOTHROW(Link());
    CHECK_NOTHROW(Link(1));
    CHECK_NOTHROW(Link(1, 100.0));
    

    std::map<fns::Band, std::vector<std::vector<int>>> bandSlotMatrix = {
        {fns::Band::C, {{320}, {320}}}, // 2 cores, 1 mode each
        {fns::Band::L, {{200}, {200}}}
    };
    auto fiber1 = std::make_shared<Fiber>(bandSlotMatrix);
    CHECK_NOTHROW(Link(1, 100.0, fiber1));

    auto fiber2 = std::make_shared<Fiber>(bandSlotMatrix);
    auto fiber3 = std::make_shared<Fiber>(bandSlotMatrix);
    std::vector<std::shared_ptr<Fiber>> fibers = {fiber2, fiber3};
    CHECK_NOTHROW(Link(1, 100.0, fibers));

    // Invalid constructors
    CHECK_THROWS_AS(Link(1, -50.0), std::invalid_argument);
    CHECK_THROWS_AS(Link(1, 0.0, fiber1), std::invalid_argument);
    std::shared_ptr<Fiber> nullFiber = nullptr;
    CHECK_THROWS_AS(Link(1, 100.0, nullFiber), std::invalid_argument);
    std::vector<std::shared_ptr<Fiber>> emptyFibers;
    CHECK_THROWS_AS(Link(1, 100.0, emptyFibers), std::invalid_argument);
    std::vector<std::shared_ptr<Fiber>> fibersWithNull = {fiber1, nullptr};
    CHECK_THROWS_AS(Link(1, 100.0, fibersWithNull), std::invalid_argument);

    // Check default values
    Link defaultLink;
    CHECK(defaultLink.getId() == -1);
    CHECK(defaultLink.getLength() == fns::defaults::LENGTH);
    CHECK(defaultLink.getSrc() == -1);
    CHECK(defaultLink.getDst() == -1);
    CHECK(defaultLink.getFibers().empty());

    Link link1(42);
    CHECK(link1.getId() == 42);
    CHECK(link1.getLength() == fns::defaults::LENGTH);
    CHECK(link1.getSrc() == -1);
    CHECK(link1.getDst() == -1);
    CHECK(link1.getFibers().empty());

    Link link2(10, 250.5);
    CHECK(link2.getId() == 10);
    CHECK(link2.getLength() == 250.5);
    CHECK(link2.getSrc() == -1);
    CHECK(link2.getDst() == -1);
    CHECK(link2.getFibers().empty());
}

TEST_CASE("Setters and Getters")
{
    auto link1 = Link(1, 100.0);
    CHECK_THROWS_AS(link1.setId(2), std::runtime_error); // ID already
    CHECK_THROWS_AS(link1.setLength(0.0), std::invalid_argument); // Invalid length

    CHECK_NOTHROW(link1.setLength(150.0));
    CHECK(link1.getLength() == 150.0);

    auto link2 = Link();
    CHECK_NOTHROW(link2.setId(10));
    CHECK(link2.getId() == 10);

    CHECK_NOTHROW(link1.setSrc(5));
    CHECK(link1.getSrc() == 5);
    CHECK_NOTHROW(link1.setDst(15));
    CHECK(link1.getDst() == 15);

    CHECK_THROWS_AS(link1.setSrc(-1), std::invalid_argument);
    CHECK_THROWS_AS(link1.setDst(-2), std::invalid_argument);

    auto fiber1 = std::make_shared<Fiber>(320);
    CHECK_NOTHROW(link1.addFiber(fiber1));
    CHECK_NOTHROW(link1.getFiber(0));
    CHECK_THROWS_AS(link1.getFiber(1), std::out_of_range);
    CHECK(link1.getNumberOfFibers() == 1);

    auto fiber2 = std::make_shared<Fiber>(200);
    link1.addFiber(fiber2);
    CHECK(link1.getNumberOfFibers() == 2);
}

TEST_CASE("addCable Method")
{
    Link link;
    
    // Test adding different fiber types
    CHECK_NOTHROW(link.addCable(fns::FiberType::SSMF, 2));
    CHECK(link.getNumberOfFibers() == 2);
    CHECK(link.getFiber(0)->getType() == fns::FiberType::SSMF);
    CHECK(link.getFiber(1)->getType() == fns::FiberType::SSMF);
    
    CHECK_NOTHROW(link.addCable(fns::FiberType::MCF, 1));
    CHECK(link.getNumberOfFibers() == 3);
    CHECK(link.getFiber(2)->getType() == fns::FiberType::MCF);
    
    CHECK_NOTHROW(link.addCable(fns::FiberType::FMF, 1));
    CHECK(link.getNumberOfFibers() == 4);
    CHECK(link.getFiber(3)->getType() == fns::FiberType::FMF);

    CHECK_NOTHROW(link.addCable(fns::FiberType::FMMCF, 1));
    CHECK(link.getNumberOfFibers() == 5);
    CHECK(link.getFiber(4)->getType() == fns::FiberType::FMMCF);
    
    // Test invalid operations
    CHECK_THROWS_AS(link.addCable(fns::FiberType::SSMF, 0), std::invalid_argument);
    CHECK_THROWS_AS(link.addCable(fns::FiberType::SSMF, -1), std::invalid_argument);
}


// TODO: Add usage percentage tests