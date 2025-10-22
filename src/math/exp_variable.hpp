#ifndef __EXP_VARIABLE_H__
#define __EXP_VARIABLE_H__

#include <stdexcept>

#include "random_variable.hpp"

/**
 * @brief Class that generates values from an exponential distribution.
 *
 * This class inherits from RandomVariable and generates random numbers
 * following an exponential distribution.
 */
class ExpVariable : public RandomVariable {
 public:
  /**
   * @brief Constructs a new ExpVariable object.
   *
   * @param seed The seed for the random number generator.
   * @param lambda The rate parameter (lambda) of the exponential distribution.
   */
  ExpVariable(unsigned int seed, double lambda);

  /**
   * @brief Generates the next random value from the exponential distribution.
   *
   * @return double The next random value.
   */
  double getNextValue(void) override;

 private:
  std::exponential_distribution<double> dist;
};

#endif