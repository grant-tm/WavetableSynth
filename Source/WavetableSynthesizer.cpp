#include "WavetableSynthesizer.h"

WavetableSynthesizer::WavetableSynthesizer()
{
	wavetable.setSize(0, 0);
}

WavetableSynthesizer::~WavetableSynthesizer()
{
	wavetable.clear();
}

void WavetableSynthesizer::setWavetable(Wavetable& wavetableToCopy)
{
	wavetable = wavetableToCopy;
}

const Wavetable *WavetableSynthesizer::getWavetableReadPointer()
{
	return &wavetable;
}

int WavetableSynthesizer::getNumWavetableFrames()
{
	return wavetable.getNumChannels();
}