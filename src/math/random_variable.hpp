#ifndef __RANDOM_VARIABLE_H__
#define __RANDOM_VARIABLE_H__

#include <random>

/**
 * @brief An abstract base class for random variable generators.
 *
 * This class provides a common interface for different types of random
 * variable distributions. It is intended to be inherited by specific
 * distribution classes like UniformVariable, ExpVariable, and
 * NormalVariable.
 */
class RandomVariable {
 public:
  /**
   * @brief Constructs a new RandomVariable object.
   *
   * @param seed The seed for the random number generator.
   */
  RandomVariable(unsigned int seed);

  /**
   * @brief Virtual destructor.
   */
  virtual ~RandomVariable() = default;

  /**
   * @brief Generates the next random value from the distribution.
   *
   * This is a pure virtual function that must be implemented by any concrete
   * subclass.
   *
   * @return double The next random value.
   */
  virtual double getNextValue(void) = 0;

 protected:
  /**
   * @brief The underlying random number engine.
   */
  std::mt19937 generator;
};

#endif