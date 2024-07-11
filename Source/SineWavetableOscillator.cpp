#include "SineWavetableOscillator.h"

SineWavetableOscillator::SineWavetableOscillator() : WavetableOscillator() {
	generateSineTable(128);
};

void SineWavetableOscillator::generateSineTable(int resolution)
{
	wavetable.setSize(1, resolution);
	wavetableSize = resolution;

	auto* samples = wavetable.getWritePointer(0);

	auto angleDelta = juce::MathConstants<float>::twoPi / (float) (resolution - 1);
	auto currentAngle = 0.0;

	for (int i = 0; i < resolution; ++i)
	{
		auto sample = std::sin(currentAngle);
		samples[i] = (float) sample;
		currentAngle += angleDelta;
	}
}