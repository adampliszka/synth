/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ChorusFlangerAudioProcessorEditor::ChorusFlangerAudioProcessorEditor (ChorusFlangerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize(400, 300);

    addAndMakeVisible(modeSelector);
    modeSelector.addItem("Chorus", 1);
    modeSelector.addItem("Flanger", 2);
    modeSelector.setSelectedId(1, juce::dontSendNotification);

    setupKnob(rateKnob, rateLabel, "Rate", 0.1f, 20.0f, 5.0f);
    setupKnob(depthKnob, depthLabel, "Depth", 0.0f, 1.0f, 0.6f);
    setupKnob(delayKnob, delayLabel, "Delay", 1.0f, 25.0f, 15.0f);
    setupKnob(feedbackKnob, feedbackLabel, "Feedback", 0.0f, 1.0f, 0.05f);


	modeSelector.addListener(this);
    rateKnob.addListener(this);
    depthKnob.addListener(this);
    delayKnob.addListener(this);
    feedbackKnob.addListener(this);

    //comboBoxChanged(&modeSelector);
}
ChorusFlangerAudioProcessorEditor::~ChorusFlangerAudioProcessorEditor()
{
	modeSelector.removeListener(this);
    rateKnob.removeListener(this);
    depthKnob.removeListener(this);
    delayKnob.removeListener(this);
    feedbackKnob.removeListener(this);
}

void ChorusFlangerAudioProcessorEditor::setupKnob(juce::Slider& knob, juce::Label& label, const juce::String& name, float min, float max, float defaultVal)
{
    addAndMakeVisible(knob);
    knob.setSliderStyle(juce::Slider::Rotary);
    knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    knob.setRange(min, max, 0.01f);
    knob.setValue(defaultVal);

    addAndMakeVisible(label);
    label.setText(name, juce::dontSendNotification);
    label.attachToComponent(&knob, false);
    label.setJustificationType(juce::Justification::centred);
}

void ChorusFlangerAudioProcessorEditor::comboBoxChanged(juce::ComboBox* comboBox)
{
    bool isChorus = (modeSelector.getSelectedId() == 1);
    audioProcessor.isChorus = isChorus;

    float oldRate = rateKnob.getValue();
    float oldDelay = delayKnob.getValue();
    float oldFeedback = feedbackKnob.getValue();
    rateKnob.setRange(isChorus ? 0.1f : 0.2f, 20.0f, 0.01f);
    depthKnob.setSkewFactor(isChorus ? 1.0f : 1.2f);
    delayKnob.setRange(isChorus ? 1.0f : 0.5f, isChorus ? 25.0f : 5.0f, 0.01f);
	feedbackKnob.setValue(isChorus ? oldFeedback / 3 : oldFeedback * 3, juce::dontSendNotification);

    
    rateKnob.setValue(isChorus ? juce::jmap(oldRate, 0.2f, 20.0f, 0.1f, 20.0f) : juce::jmap(oldRate, 0.1f, 20.0f, 0.2f, 20.0f), juce::dontSendNotification);

    delayKnob.setValue(isChorus ? juce::jmap(oldDelay, 0.5f, 5.0f, 1.0f, 25.0f) : juce::jmap(oldDelay, 1.0f, 25.0f, 0.5f, 5.0f), juce::dontSendNotification);

    sliderValueChanged(&rateKnob);
	sliderValueChanged(&depthKnob);
	sliderValueChanged(&delayKnob);
	sliderValueChanged(&feedbackKnob);
}

void ChorusFlangerAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    audioProcessor.rate = rateKnob.getValue();
    audioProcessor.depth = depthKnob.getValue();
    audioProcessor.delay = delayKnob.getValue();
    audioProcessor.feedback = feedbackKnob.getValue();
}




//==============================================================================
void ChorusFlangerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    
}

void ChorusFlangerAudioProcessorEditor::resized()
{
    modeSelector.setBounds(10, 10, getWidth() - 20, 30);

    int knobWidth = 70;
    int knobHeight = 70;
    int knobSpacing = 20;
    int startX = 10;
    int startY = 80;

    rateKnob.setBounds(startX, startY, knobWidth, knobHeight);
    depthKnob.setBounds(startX + knobWidth + knobSpacing, startY, knobWidth, knobHeight);
    delayKnob.setBounds(startX + 2 * (knobWidth + knobSpacing), startY, knobWidth, knobHeight);
    feedbackKnob.setBounds(startX + 3 * (knobWidth + knobSpacing), startY, knobWidth, knobHeight);
}
