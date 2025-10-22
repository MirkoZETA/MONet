// Let Catch provide main():
#define CATCH_CONFIG_MAIN

#include "../src/core/node.hpp"
#include "catch.hpp"

TEST_CASE("Constructor (Node)")
{
    CHECK_NOTHROW(Node());
    CHECK_NOTHROW(Node(0, std::optional<std::string>()));
    CHECK_NOTHROW(Node(1, "Test node 1"));
    CHECK_NOTHROW(Node(1, 5, 3, "Test node with DCs, IXPs and labels"));
    CHECK_NOTHROW(Node(1, 5, 3, 2000.0, "Test node with all params", -74.00597, 40.71278, 12.34, 56.78));
}

TEST_CASE("Default Constructor (Node)")
{
    Node n;
    
    CHECK(n.getId() == -1);
    CHECK_THROWS_AS(n.getLabel(), NodeAttributeNotSetException);
    CHECK_THROWS_AS(n.getDCs(), NodeAttributeNotSetException);
    CHECK_THROWS_AS(n.getIXPs(), NodeAttributeNotSetException);
}

TEST_CASE("Constructor with ID (Node)")
{
    Node n(42, std::optional<std::string>("Test"));
    
    CHECK(n.getId() == 42);
    CHECK(n.getLabel().value() == "Test");
}

TEST_CASE("Constructor with ID and Label (Node)")
{
    Node n(10, std::optional<std::string>("Router A"));
    
    CHECK(n.getId() == 10);
    CHECK(n.getLabel().value() == "Router A");
}

TEST_CASE("Constructor with DCs and IXPs (Node)")
{
    Node n(5, 3, 2, "Core Node");
    
    CHECK(n.getId() == 5);
    CHECK(n.getDCs().value() == 3);
    CHECK(n.getIXPs().value() == 2);
    CHECK(n.getLabel().value() == "Core Node");
}

TEST_CASE("Constructor with Parameters (Node)")
{
    Node n(7, 4, 1, std::nullopt, "Edge Node", std::nullopt, std::nullopt, 12.5, 8.3);
    
    CHECK(n.getId() == 7);
    CHECK(n.getDCs().value() == 4);
    CHECK(n.getIXPs().value() == 1);
    CHECK(n.getParam1().value() == 12.5);
    CHECK(n.getParam2().value() == 8.3);
    CHECK(n.getLabel().value() == "Edge Node");
}

TEST_CASE("Copy Constructor (Node)")
{
    Node original(15, 2, 3, std::nullopt, "Original Node", std::nullopt, std::nullopt, 5.5, 7.7);
    Node copy(original);
    
    CHECK(copy.getId() == 15);
    CHECK(copy.getDCs().value() == 2);
    CHECK(copy.getIXPs().value() == 3);
    CHECK(copy.getParam1().value() == 5.5);
    CHECK(copy.getParam2().value() == 7.7);
    CHECK(copy.getLabel().value() == "Original Node");
}

TEST_CASE("Getting ID (Node)")
{
    Node n1;
    Node n2(25, std::optional<std::string>());
    
    CHECK(n1.getId() == -1);
    CHECK(n2.getId() == 25);
}

TEST_CASE("Setting ID (Node)")
{
    Node n1;
    CHECK_NOTHROW(n1.setId(100));
    CHECK(n1.getId() == 100);
    
    // Cannot set ID if already set (not -1)
    CHECK_THROWS_AS(n1.setId(200), std::runtime_error);
    
    Node n2(50, std::optional<std::string>());
    CHECK_THROWS_AS(n2.setId(60), std::runtime_error);
}

TEST_CASE("Getting and Setting Label (Node)")
{
    Node n;
    
    // Default label should throw (not set)
    CHECK_THROWS_AS(n.getLabel(), NodeAttributeNotSetException);
    
    // Set and get label
    n.setLabel("Test Router");
    CHECK(n.getLabel().value() == "Test Router");
    
    // Change label
    n.setLabel("Updated Router");
    CHECK(n.getLabel().value() == "Updated Router");
    
    // Empty label
    n.setLabel("");
    CHECK(n.getLabel().value() == "");
}

TEST_CASE("Getting DCs and IXPs (Node)")
{
    Node n1(1, 5, 3, "Test Node");
    
    CHECK(n1.getDCs().value() == 5);
    CHECK(n1.getIXPs().value() == 3);
    
    Node n2(2, 0, 0, "Minimal Node");
    CHECK(n2.getDCs().value() == 0);
    CHECK(n2.getIXPs().value() == 0);
}

TEST_CASE("Getting Parameters (Node)")
{
    Node n(1, 2, 1, std::nullopt, "Param Node", std::nullopt, std::nullopt, 15.5, -3.2);
    
    CHECK(n.getParam1().value() == 15.5);
    CHECK(n.getParam2().value() == -3.2);
}

TEST_CASE("Getting and Setting DCs (Node)")
{
    Node n;
    
    // DCs not set initially - should throw NodeAttributeNotSetException
    CHECK_THROWS_AS(n.getDCs(), NodeAttributeNotSetException);
    CHECK_THROWS_AS(n.getDCs(), std::runtime_error); // Also catchable as base class
    
    // Set and get DCs
    CHECK_NOTHROW(n.setDCs(5));
    CHECK(n.getDCs().value() == 5);
    
    // Cannot set negative DCs
    CHECK_THROWS_AS(n.setDCs(-1), std::invalid_argument);
}

TEST_CASE("Getting and Setting IXPs (Node)")
{
    Node n;
    
    // IXPs not set initially - should throw NodeAttributeNotSetException
    CHECK_THROWS_AS(n.getIXPs(), NodeAttributeNotSetException);
    CHECK_THROWS_AS(n.getIXPs(), std::runtime_error); // Also catchable as base class
    
    // Set and get IXPs
    CHECK_NOTHROW(n.setIXPs(3));
    CHECK(n.getIXPs().value() == 3);
    
    // Cannot set negative IXPs
    CHECK_THROWS_AS(n.setIXPs(-1), std::invalid_argument);
}

TEST_CASE("Getting and Setting Longitude (Node)")
{
    Node n;
    
    // Longitude not set initially
    CHECK_THROWS_AS(n.getLongitude(), NodeAttributeNotSetException);
    
    // Set and get Longitude
    CHECK_NOTHROW(n.setLongitude(-74.00597));
    CHECK(n.getLongitude().value() == Approx(-74.00597));
    
    // Invalid longitude (out of range)
    CHECK_THROWS_AS(n.setLongitude(-181.0), std::invalid_argument);
    CHECK_THROWS_AS(n.setLongitude(181.0), std::invalid_argument);
}

TEST_CASE("Getting and Setting Latitude (Node)")
{
    Node n;
    
    // Latitude not set initially
    CHECK_THROWS_AS(n.getLatitude(), NodeAttributeNotSetException);
    
    // Set and get Latitude
    CHECK_NOTHROW(n.setLatitude(40.71278));
    CHECK(n.getLatitude().value() == Approx(40.71278));
    
    // Invalid latitude (out of range)
    CHECK_THROWS_AS(n.setLatitude(-91.0), std::invalid_argument);
    CHECK_THROWS_AS(n.setLatitude(91.0), std::invalid_argument);
}

TEST_CASE("Getting and Setting Population (Node)")
{
    Node n;
    
    // Population not set initially
    CHECK_THROWS_AS(n.getPopulation(), NodeAttributeNotSetException);
    
    // Set and get Population
    CHECK_NOTHROW(n.setPopulation(1000000.0));
    CHECK(n.getPopulation().value() == Approx(1000000.0));
}

TEST_CASE("Getting and Setting Param1 (Node)")
{
    Node n;
    
    // Param1 not set initially
    CHECK_THROWS_AS(n.getParam1(), NodeAttributeNotSetException);
    
    // Set and get Param1
    CHECK_NOTHROW(n.setParam1(12.34));
    CHECK(n.getParam1().value() == Approx(12.34));
}

TEST_CASE("Getting and Setting Param2 (Node)")
{
    Node n;
    
    // Param2 not set initially
    CHECK_THROWS_AS(n.getParam2(), NodeAttributeNotSetException);
    
    // Set and get Param2
    CHECK_NOTHROW(n.setParam2(56.78));
    CHECK(n.getParam2().value() == Approx(56.78));
}

TEST_CASE("Getting and Setting Degree (Node)")
{
    Node n;
    
    // Degree not set initially (defaults to -1)
    CHECK_THROWS_AS(n.getDegree(), NodeAttributeNotSetException);
    
    // Set and get Degree
    CHECK_NOTHROW(n.setDegree(4));
    CHECK(n.getDegree() == 4);
    
    // Cannot set negative degree
    CHECK_THROWS_AS(n.setDegree(-2), std::invalid_argument);
}

TEST_CASE("Edge Cases (Node)")
{
    // Negative ID
    CHECK_NOTHROW(Node(-1, std::optional<std::string>()));
    CHECK_NOTHROW(Node(-100, "Negative ID"));
    
    // Large numbers
    CHECK_NOTHROW(Node(999999, 1000, 500, 1e6, "Large values", -1e6, 1e6, std::nullopt, std::nullopt));
    
    // Empty strings
    CHECK_NOTHROW(Node(1, ""));
    CHECK_NOTHROW(Node(1, 0, 0, ""));
}
