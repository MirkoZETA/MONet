#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "../src/core/demand.hpp"

TEST_CASE("Constructor (Demand)") {

  CHECK_NOTHROW(Demand());
  CHECK_NOTHROW(Demand(1, 0, 1, 10.0));

  CHECK_THROWS_AS(Demand(-1, 0, 1, 10.0), std::invalid_argument);
  CHECK_THROWS_AS(Demand(1, -1, 1, 10.0), std::invalid_argument);
  CHECK_THROWS_AS(Demand(1, 0, 0, 10.0), std::invalid_argument);
  CHECK_THROWS_AS(Demand(1, 0, 1, -5.0), std::invalid_argument);
}

TEST_CASE("Set and Get Demand Attributes") {
  Demand d(1, 0, 1, 10.0);

  CHECK(d.getId() == 1);
  CHECK(d.getSrc() == 0);
  CHECK(d.getDst() == 1);
  CHECK(d.getRequiredCapacity() == Approx(10.0).epsilon(0.01));
  CHECK(d.getAllocatedCapacity() == Approx(0.0).epsilon(0.01));
  CHECK(d.getUnprovisionedCapacity() == Approx(10.0).epsilon(0.01));
  CHECK(d.isProvisioned() == false);
  CHECK(d.getProvisioningRatio() == Approx(0.0).epsilon(0.01));
  CHECK(d.isNull() == false);

  Demand nullDemand;
  CHECK(nullDemand.isNull() == true);
  CHECK(nullDemand.getId() == -1);
  CHECK(nullDemand.getSrc() == -1);
  CHECK(nullDemand.getDst() == -1);

  nullDemand.setRequiredCapacity(20.0);
  CHECK(nullDemand.getRequiredCapacity() == Approx(20.0).epsilon(0.01));
  nullDemand.setId(2);
  CHECK(nullDemand.getId() == 2);
  nullDemand.setSrc(1);
  CHECK(nullDemand.getSrc() == 1);
  nullDemand.setDst(2);
  CHECK(nullDemand.getDst() == 2);
  CHECK(nullDemand.isNull() == false);
  CHECK_THROWS_AS(nullDemand.setRequiredCapacity(-5.0), std::invalid_argument);
  CHECK_THROWS_AS(nullDemand.setId(-1), std::invalid_argument);
  CHECK_THROWS_AS(nullDemand.setSrc(-1), std::invalid_argument);
  CHECK_THROWS_AS(nullDemand.setDst(-1), std::invalid_argument);
}

TEST_CASE("Demand operation") {
  Demand d(1, 0, 1, 10.0);
  d.addAllocatedCapacity(15.0);
  CHECK(d.isProvisioned() == true);
  CHECK(d.getUnprovisionedCapacity() == Approx(0.0).epsilon(0.01));

  d.subtractAllocatedCapacity(6.0);
  CHECK(d.isProvisioned() == false);
}