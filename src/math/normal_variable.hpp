#ifndef __NORMAL_VARIABLE_H__
#define __NORMAL_VARIABLE_H__

#include "random_variable.hpp"

/**
 * @brief Class that generates values from a normal distribution.
 *
 * This class inherits from RandomVariable and generates random numbers
 * following a normal (or Gaussian) distribution.
 */
class NormalVariable : public RandomVariable {
 public:
  /**
   * @brief Default constructor for a new NormalVariable object.
   */
  NormalVariable();
  /**
   * @brief Constructs a new NormalVariable object.
   *
   * @param seed The seed for the random number generator.
   * @param mean The mean (mu) of the normal distribution.
   * @param stddev The standard deviation (sigma) of the normal distribution.
   */
  NormalVariable(unsigned int seed, double mean, double stddev);

  /**
   * @brief Generates the next random value from the normal distribution.
   *
   * @return double The next random value.
   */
  double getNextValue(void) override;

  /**
   * @brief Gets the last generated random value from the normal distribution.
   *
   * @return double The last generated random value.
   */
  double getCurrentValue(void);

 private:
  double currentValue;
  std::normal_distribution<double> dist;
};

#endif