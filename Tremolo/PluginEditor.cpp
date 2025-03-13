/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TremoloAudioProcessorEditor::TremoloAudioProcessorEditor(TremoloAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    addAndMakeVisible(depthSlider);
    depthSlider.setRange(0.0, 1.0);
    depthSlider.setValue(*audioProcessor.depth);
    depthSlider.onValueChange = [this] { *audioProcessor.depth = depthSlider.getValue(); };

    addAndMakeVisible(rateSlider);
    rateSlider.setRange(0.1, 10.0);
    rateSlider.setValue(*audioProcessor.rate);
    rateSlider.onValueChange = [this] { *audioProcessor.rate = rateSlider.getValue(); };

    addAndMakeVisible(depthLabel);
    depthLabel.setText("Depth", juce::dontSendNotification);
    depthLabel.attachToComponent(&depthSlider, true);

    addAndMakeVisible(rateLabel);
    rateLabel.setText("Rate", juce::dontSendNotification);
    rateLabel.attachToComponent(&rateSlider, true);

    setSize(400, 150);
}

TremoloAudioProcessorEditor::~TremoloAudioProcessorEditor() {}

void TremoloAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(15.0f));
}
void TremoloAudioProcessorEditor::resized()
{
    depthSlider.setBounds(40, 30, 320, 20);
    rateSlider.setBounds(40, 60, 320, 20);
}
