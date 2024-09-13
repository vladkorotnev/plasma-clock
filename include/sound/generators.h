#pragma once
#include <sound/waveout.h>

class ToneGenerator {
public:
    enum Parameter {
        PARAMETER_FREQUENCY,
        PARAMETER_DUTY,
        PARAMETER_ACTIVE,
        PARAMETER_SAMPLE_ADDR,
    };
    virtual size_t generate_samples(void* buffer, size_t length, uint32_t wanted_samples) { return 0; }
    virtual void set_parameter(Parameter p, int v) { }
    virtual int get_parameter(Parameter p) { return 0; }
    virtual void reset_phase() {}
};


class SquareGenerator: public ToneGenerator {
public:
    SquareGenerator();
    void set_frequency(int freq);
    void set_active(bool a);
    void reset_phase() override;
    void set_parameter(Parameter p, int v) override;
    int get_parameter(Parameter p) override;
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
    int get_parameter(Parameter p) override;

private:
    int phase;
    int wavelength;
    bool state;
    bool active;
    uint32_t rng;
};

typedef struct rle_sample {
    const uint16_t sample_rate;
    const uint16_t root_frequency;
    /// @brief RLE data of the PWM audio sample. First byte is number of 1 bits, second byte is number of 0 bits that follow, and so forth.
    const uint8_t* rle_data;
    const size_t length;
} rle_sample_t;

class Sampler: public ToneGenerator {
public:
    Sampler();
    void set_parameter(Parameter p, int v) override;
    size_t generate_samples(void* buffer, size_t length, uint32_t want_samples_) override;
    int get_parameter(Parameter p) override;

    void load_sample(const rle_sample_t *);
private:
    bool active;
    bool state;
    const rle_sample_t * waveform;
    int playhead;
    int remaining_samples;
    int stretch_factor;
    int frequency;
    void rewind();
};
