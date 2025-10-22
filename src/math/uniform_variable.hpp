#ifndef __UNIFORM_VARIABLE_H__
#define __UNIFORM_VARIABLE_H__

#include <stdexcept>

#include "random_variable.hpp"

/**
 * @brief Class that generates values from a uniform distribution.
 *
 * This class inherits from RandomVariable and generates random numbers
 * following a uniform distribution.
 */
class UniformVariable : public RandomVariable {
 public:
  /**
   * @brief Constructs a new UniformVariable object.
   *
   * @param seed The seed for the random number generator.
   * @param min The minimum value of the distribution.
   * @param max The maximum value of the distribution.
   */
  UniformVariable(unsigned int seed, double min, double max);

  /**
   * @brief Generates the next random value from the uniform distribution.
   *
   * @return double The next random value.
   */
  double getNextValue(void) override;

 private:
  std::uniform_real_distribution<double> dist;
};

#endif