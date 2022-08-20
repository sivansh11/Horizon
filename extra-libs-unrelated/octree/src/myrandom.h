#ifndef MYRANDOM_H
#define MYRANDOM_H

#include <random>

class RandomNumberGenerator
{
public:
    RandomNumberGenerator() { randomNumberGenerator.seed(0); }
    RandomNumberGenerator(uint32_t seed) { init(seed); }

    void init(uint32_t seed) { randomNumberGenerator.seed(seed); }

    inline float randomFloat() 
    {
        return distribution(randomNumberGenerator);
    }

    inline float randomFloat(float min, float max)
    {
        return min + (max - min) * randomFloat();
    }

private:
    std::mt19937 randomNumberGenerator;
    std::uniform_real_distribution<float> distribution {0.f, 1.f};
};

#endif