#include "normal_variable.hpp"

NormalVariable::NormalVariable()
    : RandomVariable(0), dist(0.0, 1.0) {
  this->currentValue = 0.0;
}

NormalVariable::NormalVariable(unsigned int seed, double mean, double stddev)
    : RandomVariable(seed), dist(mean, stddev) {
  this->currentValue = mean;
}

double NormalVariable::getNextValue(void) {
  this->currentValue = this->dist(this->generator);
  return this->currentValue;
}

double NormalVariable::getCurrentValue(void) { return this->currentValue; }