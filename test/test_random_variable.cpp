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

TEST_CASE("68-95-99.7 Rule (Normal)")
{
    double mu = 0.0;       // mean
    double sigma = 1.0;    // standard deviation
    NormalVariable n(11111, mu, sigma);
    
    int number_of_samples = 1000000;
    int within_1_sigma = 0;
    int within_2_sigma = 0;
    
    for (int i = 0; i < number_of_samples; i++) {
        double sample = n.getNextValue();
        double distance_from_mean = fabs(sample - mu);
        
        if (distance_from_mean <= 1.0 * sigma) within_1_sigma++;
        if (distance_from_mean <= 2.0 * sigma) within_2_sigma++;
    }
    
    double percent_1_sigma = (double)within_1_sigma / number_of_samples * 100.0;
    double percent_2_sigma = (double)within_2_sigma / number_of_samples * 100.0;
    
    // The famous 68-95-99.7 rule - unique to normal distribution!
    CHECK(percent_1_sigma == Approx(68.27).epsilon(0.02));  // ~68%
    CHECK(percent_2_sigma == Approx(95.45).epsilon(0.02));  // ~95%
}

TEST_CASE("Symmetry (Normal)")
{
    double mu = 0.0;       // mean at zero for symmetry test
    double sigma = 2.0;    // standard deviation
    NormalVariable n(22222, mu, sigma);
    
    int number_of_samples = 500000;
    int above_mean = 0;
    
    for (int i = 0; i < number_of_samples; i++) {
        double sample = n.getNextValue();
        if (sample > mu) above_mean++;
    }
    
    double percent_above = (double)above_mean / number_of_samples * 100.0;
    
    // Normal distribution is perfectly symmetric - 50% above, 50% below mean
    CHECK(percent_above == Approx(50.0).epsilon(0.02));  // ~50%
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
