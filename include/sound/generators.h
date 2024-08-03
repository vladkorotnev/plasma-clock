#pragma once
#include <sound/waveout.h>

class ToneGenerator {
public:
    enum Parameter {
        PARAMETER_FREQUENCY,
        PARAMETER_DUTY,
        PARAMETER_ACTIVE
    };
    virtual size_t generate_samples(void* buffer, size_t length, uint32_t wanted_samples) { return 0; }
    virtual void set_parameter(Parameter p, int v) { }
    virtual void reset_phase() {}
};


class SquareGenerator: public ToneGenerator {
public:
    SquareGenerator();
    void set_frequency(int freq);
    void set_active(bool a);
    void reset_phase() override;
    void set_parameter(Parameter p, int v) override;
    size_t generate_samples(void* buffer, size_t length, uint32_t want_samples_) override;

private:
    int phase;
    int wavelength;
    int duty;
    bool active;
};


class NoiseGenerator: public ToneGenerator {
public:
    NoiseGenerator();
    void set_parameter(Parameter p, int v) override;
    size_t generate_samples(void* buffer, size_t length, uint32_t want_samples_) override;

private:
    int phase;
    int wavelength;
    bool state;
    bool active;
    uint32_t rng;
};
