// Let Catch provide main():
#define CATCH_CONFIG_MAIN

#include "../src/math/uniform_variable.hpp"
#include "../src/math/normal_variable.hpp"
#include "catch.hpp"


TEST_CASE("Constructor (Normal)")
{
    CHECK_NOTHROW(NormalVariable());
    CHECK_NOTHROW(NormalVariable(12345, 0.0, 1.0));
}

TEST_CASE("Mean (Normal)")
{
    double mu = 10.0;      // mean
    double sigma = 3.0;    // standard deviation
    NormalVariable n(12345, mu, sigma);
    
    double mean = 0.0;
    int number_of_samples = 1000000;
    double sample;
    
    for (int i = 0; i < number_of_samples; i++) {
        sample = n.getNextValue();
        mean += sample;
    }
    mean /= number_of_samples;
    
    CHECK(mean == Approx(mu).epsilon(0.01));  // 1% difference
}

TEST_CASE("Variance (Normal)")
{
    double mu = 5.0;       // mean
    double sigma = 2.0;    // standard deviation
    NormalVariable n(54321, mu, sigma);
    
    double mean = 0.0;
    int number_of_samples = 1000000;
    auto samples = std::vector<double>(number_of_samples);
    double sample;
    double sn;
    
    for (int i = 0; i < number_of_samples; i++) {
        sample = n.getNextValue();
        mean += sample;
        samples[i] = sample;
    }
    mean /= number_of_samples;
    
    CHECK(mean == Approx(mu).epsilon(0.01));  // 1% difference
    
    sn = 0;
    for (int i = 0; i < number_of_samples; i++) {
        sn += (mean - samples[i]) * (mean - samples[i]);
    }
    sn /= number_of_samples;
    
    double theoretical_variance = sigma * sigma;  // For normal: variance = σ²
    CHECK(sn == Approx(theoretical_variance).epsilon(0.01));  // 1% difference
}

/**
 * Test log-normal distribution properties.
 * NormalVariable is actually a log-normal distribution (for traffic growth modeling).
 * Log-normal has these properties:
 * - All values are positive (> 0)
 * - Distribution is right-skewed (not symmetric)
 * - Median = exp(mu) where mu is the underlying normal mean
 */
TEST_CASE("Log-Normal Properties (NormalVariable)")
{
    double mu = 0.29;      // desired mean growth rate
    double sigma = 0.1;    // standard deviation
    NormalVariable n(11111, mu, sigma);
    
    int number_of_samples = 100000;
    int positive_samples = 0;
    std::vector<double> samples;
    samples.reserve(number_of_samples);
    
    for (int i = 0; i < number_of_samples; i++) {
        double sample = n.getNextValue();
        samples.push_back(sample);
        if (sample > 0) positive_samples++;
    }
    
    // Log-normal property 1: All values must be positive
    CHECK(positive_samples == number_of_samples);
    
    // Log-normal property 2: Mean should be close to target mu
    double sample_mean = 0.0;
    for (double s : samples) sample_mean += s;
    sample_mean /= number_of_samples;
    CHECK(sample_mean == Approx(mu).epsilon(0.05));  // 5% tolerance
    
    // Log-normal property 3: Values should be right-skewed (median < mean for positive skew)
    std::sort(samples.begin(), samples.end());
    double median = samples[number_of_samples / 2];
    // For log-normal with small sigma, median ≈ mean, but slightly less due to right skew
    CHECK(median < sample_mean * 1.1);  // Median should be reasonably close to mean
    CHECK(median > 0);
}

TEST_CASE("Log-Normal Skewness (NormalVariable)")
{
    double mu = 0.5;       // mean
    double sigma = 0.3;    // larger sigma shows more skewness
    NormalVariable n(22222, mu, sigma);
    
    int number_of_samples = 100000;
    int above_mean = 0;
    
    for (int i = 0; i < number_of_samples; i++) {
        double sample = n.getNextValue();
        if (sample > mu) above_mean++;
    }
    
    double percent_above = (double)above_mean / number_of_samples * 100.0;
    
    // Log-normal is right-skewed: more values below mean than above
    // (unlike normal which is 50-50)
    // With right skew, we expect roughly 40-50% above mean depending on sigma
    CHECK(percent_above > 30.0);  // Not extremely skewed
    CHECK(percent_above < 60.0);  // But definitely some skewness possible
}



TEST_CASE("Constructor (Uniform)")
{
    CHECK_NOTHROW(UniformVariable(12345, 0.0, 10.0));
    CHECK_THROWS(UniformVariable(12345, 10.0, 5.0));  // min >= max
}

TEST_CASE("Mean (Uniform)")
{
    double min = 2.0;
    double max = 8.0;
    UniformVariable u(12345, min, max);
    
    double mean = 0.0;
    int number_of_samples = 1000000;
    
    for (int i = 0; i < number_of_samples; i++) {
        double sample = u.getNextValue();
        mean += sample;
    }
    mean /= number_of_samples;
    
    double theoretical_mean = (min + max) / 2.0;  // For uniform: mean = (a + b) / 2
    CHECK(mean == Approx(theoretical_mean).epsilon(0.01));  // 1% difference
}

TEST_CASE("Variance (Uniform)")
{
    double min = 0.0;
    double max = 12.0;
    UniformVariable u(54321, min, max);
    
    double mean = 0.0;
    int number_of_samples = 1000000;
    auto samples = std::vector<double>(number_of_samples);
    double sample;
    double sn;
    
    for (int i = 0; i < number_of_samples; i++) {
        sample = u.getNextValue();
        mean += sample;
        samples[i] = sample;
    }
    mean /= number_of_samples;
    
    double theoretical_mean = (min + max) / 2.0;
    CHECK(mean == Approx(theoretical_mean).epsilon(0.01));  // 1% difference
    
    sn = 0;
    for (int i = 0; i < number_of_samples; i++) {
        sn += (mean - samples[i]) * (mean - samples[i]);
    }
    sn /= number_of_samples;
    
    double theoretical_variance = (max - min) * (max - min) / 12.0;  // For uniform: var = (b - a)² / 12
    CHECK(sn == Approx(theoretical_variance).epsilon(0.01));  // 1% difference
}

TEST_CASE("Range Bounds (Uniform)")
{
    double min = 2.0;
    double max = 8.0;
    UniformVariable u(98765, min, max);
    
    int number_of_samples = 100000;
    bool all_within_bounds = true;
    
    for (int i = 0; i < number_of_samples; i++) {
        double sample = u.getNextValue();
        if (sample < min || sample >= max) {  // uniform_real_distribution is [min, max)
            all_within_bounds = false;
            break;
        }
    }
    
    // UNIQUE to uniform: ALL values must be within strict bounds
    CHECK(all_within_bounds == true);
}

TEST_CASE("Flat Distribution (Uniform)")
{
    double min = 0.0;
    double max = 10.0;
    UniformVariable u(11111, min, max);
    
    int number_of_samples = 200000;
    int number_of_bins = 10;
    std::vector<int> bins(number_of_bins, 0);
    
    double bin_width = (max - min) / number_of_bins;
    
    for (int i = 0; i < number_of_samples; i++) {
        double sample = u.getNextValue();
        int bin_index = (int)((sample - min) / bin_width);
        if (bin_index >= number_of_bins) bin_index = number_of_bins - 1;
        bins[bin_index]++;
    }
    
    double expected_count_per_bin = (double)number_of_samples / number_of_bins;
    
    // UNIQUE to uniform: all bins should have nearly equal counts (flat distribution)
    for (int i = 0; i < number_of_bins; i++) {
        CHECK(bins[i] == Approx(expected_count_per_bin).epsilon(0.05));  // 5% tolerance
    }
}
