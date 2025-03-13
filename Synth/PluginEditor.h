/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class DecibelSlider : public juce::Slider
{
    double minDb = -60.0;
    double maxDb = 0.0;
public:
    DecibelSlider() {}
    ~DecibelSlider() {}
    juce::String getTextFromValue(double value) override
    {
        return juce::Decibels::toString(juce::Decibels::gainToDecibels(value), 2, minDb);
    }
    double getValueFromText(const juce::String& text) override
    {
        return juce::Decibels::decibelsToGain(text.getDoubleValue(), minDb);
    }
    double valueToProportionOfLength(double value) override
    {
        return juce::jmap(juce::jlimit(minDb, maxDb, juce::Decibels::gainToDecibels(value, minDb)), minDb, maxDb, 0.0, 1.0);
    };
    double proportionOfLengthToValue(double proportion) override
    {
        return juce::Decibels::decibelsToGain(juce::jmap(proportion, 0.0, 1.0, minDb, maxDb), minDb);
    };
};

//==============================================================================
/**
*/
class SynthAudioProcessorEditor : public juce::AudioProcessorEditor,
    public juce::Slider::Listener,
    public juce::ComboBox::Listener,
    public juce::Button::Listener
{
public:
    SynthAudioProcessorEditor(SynthAudioProcessor&);
    ~SynthAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* comboBox) override;
    void buttonClicked(juce::Button* button) override;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SynthAudioProcessor& audioProcessor;

    DecibelSlider gain;
    juce::Slider pulseWidth;
    juce::ComboBox shape;
    juce::ToggleButton autoWahButton;

    juce::Slider attack;
    juce::Slider decay;
    juce::Slider sustain;
    juce::Slider release;

    // Auto-wah controls
    juce::Slider autoWahFrequency;
    juce::Slider autoWahDepth;
    juce::Slider autoWahRate;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthAudioProcessorEditor)
};
