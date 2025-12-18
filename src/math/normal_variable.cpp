#include "normal_variable.hpp"

NormalVariable::NormalVariable()
    : RandomVariable(0), dist(0.0, 1.0) {
  this->currentValue = 0.0;
}

NormalVariable::NormalVariable(unsigned int seed, double mean, double stddev)
    : RandomVariable(seed) {
  // Convert desired mean (M) and stddev (S) to lognormal parameters (mu, sigma)
  // sigma^2 = ln(1 + S^2/M^2)
  // mu = ln(M) - sigma^2/2
  double variance = stddev * stddev;
  double meanSq = mean * mean;
  double sigmaSq = std::log(1.0 + variance / meanSq);
  double sigma = std::sqrt(sigmaSq);
  double mu = std::log(mean) - sigmaSq / 2.0;
  
  this->dist = std::lognormal_distribution<double>(mu, sigma);
  this->currentValue = mean;
}

double NormalVariable::getNextValue(void) {
  this->currentValue = this->dist(this->generator);
  return this->currentValue;
}

double NormalVariable::getCurrentValue(void) { return this->currentValue; }