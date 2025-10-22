#include "uniform_variable.hpp"

UniformVariable::UniformVariable(unsigned int seed, double min, double max)
    : RandomVariable(seed), dist(min, max) {
  if (min >= max) {
    throw std::runtime_error("Min value must be less than max value.");
  }
}

double UniformVariable::getNextValue(void) { return this->dist(this->generator); }
