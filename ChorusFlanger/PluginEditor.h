/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class ChorusFlangerAudioProcessorEditor  : public juce::AudioProcessorEditor,
	public juce::Slider::Listener,
	public juce::ComboBox::Listener
{
public:
    ChorusFlangerAudioProcessorEditor (ChorusFlangerAudioProcessor&);
    ~ChorusFlangerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
	void setupKnob(juce::Slider& knob, juce::Label& label, const juce::String& name, float min, float max, float defaultVal);
	void sliderValueChanged(juce::Slider* slider);
    void comboBoxChanged(juce::ComboBox* comboBox);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
	juce::Slider rateKnob;
	juce::Slider depthKnob;
	juce::Slider delayKnob;
	juce::Slider feedbackKnob;
	juce::ComboBox modeSelector;

	juce::Label rateLabel;
	juce::Label depthLabel;
	juce::Label delayLabel;
	juce::Label feedbackLabel;

    ChorusFlangerAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChorusFlangerAudioProcessorEditor)
};
