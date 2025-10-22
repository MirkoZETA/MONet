#include "random_variable.hpp"

RandomVariable::RandomVariable(unsigned int seed) {
  this->generator = std::mt19937(seed);
}
