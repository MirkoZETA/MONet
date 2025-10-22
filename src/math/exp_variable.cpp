#include "exp_variable.hpp"

ExpVariable::ExpVariable(unsigned int seed, double lambda)
    : RandomVariable(seed), dist(lambda) {
  if (lambda <= 0) {
    throw std::runtime_error("Lambda parameter must be positive.");
  }
}

double ExpVariable::getNextValue() {
  return this->dist(this->generator);
}
