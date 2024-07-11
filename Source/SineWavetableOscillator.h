#ifndef SINE_WAVETABLE_OSCILLATOR_H
#define SINE_WAVETABLE_OSCILLATOR_H

#include <JuceHeader.h>

#include "WavetableOscillator.h"

class SineWavetableOscillator : public WavetableOscillator
{
public:
	SineWavetableOscillator();
	void generateSineTable(int);
};

#endif // SINE_WAVETABLE_OSCILLATOR_H