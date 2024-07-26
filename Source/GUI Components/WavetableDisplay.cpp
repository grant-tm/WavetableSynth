#include "WavetableDisplay.h"

//================================================================================================
// CONSTRUCTORS / DESTRUCTORS

WavetableDisplayComponent::WavetableDisplayComponent(WavetableSynthAudioProcessor &p) :
    audioProcessor(p)
{
    wavetableRef = audioProcessor.synthesizer.getWavetableReadPointer();
    wavetableCurrentFrameIndex = 0;

    const auto& params = audioProcessor.getParameters();
    for (auto param : params)
    {
        param->addListener(this);
    }

    startTimerHz(24);
}

WavetableDisplayComponent::~WavetableDisplayComponent()
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params)
    {
        param->removeListener(this);
    }
}

//================================================================================================
// COMPONENT OVERRIDES

void WavetableDisplayComponent::paint(juce::Graphics& g)
{
    using namespace juce;

    //------------------------------------------------------------------------
    // PAINT BACKGROUND
    g.fillAll(Colour(SCREEN_MAIN_COLOR_HEX));

    //------------------------------------------------------------------------
    // PAINT WAVETABLE WAVE
    
    Path wavetableCurve = createPathFromWavetable();
    
    // draw shadow
    g.setColour(Colour(SCREEN_SHADOW_COLOR_HEX));
    g.fillPath(wavetableCurve);

    // draw 0 line
    Path lineLevel = createLineLevelPath();
    g.setColour(Colour(0xFF97C6AE));
    g.strokePath(lineLevel, PathStrokeType(1.5f));
    
    // draw line
    g.setColour(Colour(BORDER_COLOR_HEX));
    g.strokePath(wavetableCurve, PathStrokeType(3.f));

    //------------------------------------------------------------------------
    // PAINT BORDER

    auto border = getLocalBounds().toFloat();
    g.setColour(Colour(BORDER_COLOR_HEX));
    g.drawRect(border, 5.f);
}

void WavetableDisplayComponent::resized()
{
   
}

//================================================================================================
// WAVETABLE UPDATE SIGNAL

void WavetableDisplayComponent::parameterValueChanged(int parameterIndex, float newValue)
{
    juce::ignoreUnused(parameterIndex, newValue);
    wavetableChanged.set(true);
}

void WavetableDisplayComponent::updateWavetable()
{   
    int resolution = 512;
    //generateMultiSineWavetable(wavetable, resolution, 2, 5);
    generateSawWavetable(wavetable, resolution);
}

//================================================================================================
// TIMER SETS REFRESH RATE

void WavetableDisplayComponent::timerCallback()
{    
    if (wavetableChanged.compareAndSetBool(false, true))
    {
        wavetableCurrentFrameIndex = (int) audioProcessor.valueTree.getRawParameterValue("OSC_WAVETABLE_CURRENT_FRAME")->load();
        if (wavetableCurrentFrameIndex < 0) {
            wavetableCurrentFrameIndex = 0;
        }
        else if (wavetableCurrentFrameIndex > audioProcessor.synthesizer.getNumWavetableFrames() - 1)
        {
            wavetableCurrentFrameIndex = audioProcessor.synthesizer.getNumWavetableFrames() - 1;
        }
    }
    
    repaint();
}


//================================================================================================
// PATH PRODUERS

// calculate a single float sample by interpolating around the current sampleIndex and sampleOffset
float WavetableDisplayComponent::getHermiteInterpolatedWavetableSample(float phase)
{
    // get sample index and offset
    int wavetableSize = wavetableRef->getNumSamples();
    float scaledPhase = phase * wavetableSize;
    int sampleIndex = (int) scaledPhase;
    float sampleOffset = scaledPhase - (float) sampleIndex;

    // select 4 samples around sampleIndex
    auto samples = wavetableRef->getReadPointer(wavetableCurrentFrameIndex);
    float val0 = samples[(sampleIndex - 1 + wavetableSize) % wavetableSize];
    float val1 = samples[(sampleIndex + 0) % wavetableSize];
    float val2 = samples[(sampleIndex + 1) % wavetableSize];
    float val3 = samples[(sampleIndex + 2) % wavetableSize];

    // calculate slopes to use at points val1 and val2 (avoid discontinuities)
    float slope0 = (val2 - val0) * 0.5f;
    float slope1 = (val3 - val1) * 0.5f;

    // calculate interpolation coefficients
    float delta = val1 - val2;
    float slopeSum = slope0 + delta;
    float coefficientA = slopeSum + delta + slope1;
    float coefficientB = slopeSum + coefficientA;

    // perform interpolation
    float stage1 = coefficientA * sampleOffset - coefficientB;
    float stage2 = stage1 * sampleOffset + slope0;
    float result = stage2 * sampleOffset + val1;

    return result;
}

// calculate a single float sample by interpolating around the current sampleIndex and sampleOffset
float WavetableDisplayComponent::getLinearlyInterpolatedWavetableSample(float phase)
{
    // get sample index and offset
    int wavetableSize = wavetableRef->getNumSamples();
    float scaledPhase = phase * wavetableSize;
    int sampleIndex = static_cast<int>(scaledPhase);
    float sampleOffset = scaledPhase - static_cast<float>(sampleIndex);

    // select the samples around sampleIndex
    auto samples = wavetableRef->getReadPointer(wavetableCurrentFrameIndex);
    float val1 = samples[sampleIndex % wavetableSize];
    float val2 = samples[(sampleIndex + 1) % wavetableSize];

    // perform linear interpolation
    float result = val1 + sampleOffset * (val2 - val1);

    return result;
}

juce::Path WavetableDisplayComponent::createPathFromWavetable()
{
    //------------------------------------------------------------------------
    // COPY AUDIO BUFFER

    std::vector<float> mags;
    mags.reserve(wavetableRef->getNumSamples());
    for (int i = 0; i < wavetableRef->getNumSamples(); i++)
    {
        mags.push_back((float) wavetableRef->getSample(0, i));
    }

    //------------------------------------------------------------------------
    // INITIALIZE CALCULATION VALUES

    auto bounds = getLocalBounds().toFloat();
    const float graphYMin = bounds.getBottom();
    const float graphYMax = bounds.getY();
    const float baseYPixel = (graphYMin + graphYMax) / 2;
    const float baseXPixel = bounds.getX();
    const float numXPixels = bounds.getWidth();

    //------------------------------------------------------------------------
    // LAMBDA TO SCALE Y VALUES

    auto map = [graphYMin, graphYMax](float input)
    {
        return juce::jmap(input, -1.f, 1.f, graphYMin, graphYMax);
    };

    //------------------------------------------------------------------------
    // CALCULATE WAVETABLE PATH

    juce::Path wavetableCurve;
    wavetableCurve.startNewSubPath(baseXPixel, baseYPixel);

    for (int x = 0; x < numXPixels; ++x)
    {
        // get sapmle from wavetable
        float wavetablePhase = (float) (x / numXPixels);
        float wavetableSampleValue = getLinearlyInterpolatedWavetableSample(wavetablePhase);

        // apply scaling factor
        auto scaleCoefficient = 0.6f;
        wavetableSampleValue *= scaleCoefficient;

        // calculate pixel coordinates of next path point
        auto pixelCoordX = baseXPixel + x;
        auto pixelCoordY = map(wavetableSampleValue);

        // add point to path
        wavetableCurve.lineTo(pixelCoordX, pixelCoordY);
    }
    wavetableCurve.lineTo(baseXPixel + numXPixels, baseYPixel);

    return wavetableCurve;
}

juce::Path WavetableDisplayComponent::createLineLevelPath()
{
    juce::Path lineLevel;

    auto bounds = getLocalBounds().toFloat();
    const float graphYMin = bounds.getBottom();
    const float graphYMax = bounds.getY();
    const float baseYPixel = (graphYMin + graphYMax) / 2;
    const float baseXPixel = bounds.getX();
    const float numXPixels = bounds.getWidth();

    lineLevel.startNewSubPath(baseXPixel, baseYPixel);
    lineLevel.lineTo(baseXPixel + numXPixels, baseYPixel);
    return lineLevel;
}