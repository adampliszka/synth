#include "PluginProcessor.h"
#include "PluginEditor.h"

#define margin 10
#define sliderHeight 35
#define rotaryWidth 60
#define comboBoxHeight 30
#define autoWahExpansion 150

//==============================================================================
SynthAudioProcessorEditor::SynthAudioProcessorEditor(SynthAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setResizeLimits(480, 240, 1600, 800);
    setSize(560 * 1.1, 280 * 1.1);

    gain.setSliderStyle(juce::Slider::LinearBar);
    gain.setRange(0.0, 1.0, 0.0001);
    gain.setTextBoxStyle(juce::Slider::TextBoxBelow, false, sliderHeight, sliderHeight * 2);
    gain.setPopupDisplayEnabled(false, false, this);
    gain.setValue(audioProcessor.gain);
    addAndMakeVisible(&gain);
    gain.addListener(this);

    pulseWidth.setSliderStyle(juce::Slider::LinearBar);
    pulseWidth.setRange(0.01, 0.99, 0.01);
    pulseWidth.setTextBoxStyle(juce::Slider::TextBoxBelow, false, sliderHeight, sliderHeight * 2);
    pulseWidth.setPopupDisplayEnabled(false, false, this);
    pulseWidth.setTextValueSuffix(" Pulse Width");
    pulseWidth.setValue(audioProcessor.pulseWidth);
    addAndMakeVisible(&pulseWidth);
    pulseWidth.addListener(this);

    shape.addItem("Sine", 1);
    shape.addItem("Sawtooth", 2);
    shape.addItem("Square", 3);
    shape.addItem("Triangle", 4);
    shape.setSelectedId(audioProcessor.waveType);
    addAndMakeVisible(&shape);
    shape.addListener(this);

    autoWahButton.setButtonText("Auto-wah");
    autoWahButton.setToggleState(false, juce::dontSendNotification);
    addAndMakeVisible(&autoWahButton);
    autoWahButton.addListener(this);

    // Add auto-wah controls (hidden by default)
    autoWahFrequency.setSliderStyle(juce::Slider::Rotary);
    autoWahFrequency.setRange(300.0, 1000.0, 1.0);
    autoWahFrequency.setTextBoxStyle(juce::Slider::TextBoxBelow, false, rotaryWidth * 1.1, rotaryWidth * 1.1);
    autoWahFrequency.setPopupDisplayEnabled(false, false, this);
    autoWahFrequency.setTextValueSuffix(" Hz Frequency");
    autoWahFrequency.setValue(700.0);
    autoWahFrequency.setTextBoxIsEditable(false);
    autoWahFrequency.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 25);
    addAndMakeVisible(&autoWahFrequency);
    autoWahFrequency.setVisible(false);
    autoWahFrequency.addListener(this);

    autoWahDepth.setSliderStyle(juce::Slider::Rotary);
    autoWahDepth.setRange(0.5, 1.0, 0.01);
    autoWahDepth.setTextBoxStyle(juce::Slider::TextBoxBelow, false, rotaryWidth * 1.1, rotaryWidth * 1.1);
    autoWahDepth.setPopupDisplayEnabled(false, false, this);
    autoWahDepth.setTextValueSuffix(" Depth");
    autoWahDepth.setValue(0.8);
    autoWahDepth.setTextBoxIsEditable(false);
    autoWahDepth.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 25);
    addAndMakeVisible(&autoWahDepth);
    autoWahDepth.setVisible(false);
    autoWahDepth.addListener(this);

    autoWahRate.setSliderStyle(juce::Slider::Rotary);
    autoWahRate.setRange(1.0, 5.0, 0.1);
    autoWahRate.setTextBoxStyle(juce::Slider::TextBoxBelow, false, rotaryWidth * 1.1, rotaryWidth * 1.1);
    autoWahRate.setPopupDisplayEnabled(false, false, this);
    autoWahRate.setTextValueSuffix(" Hz Rate");
    autoWahRate.setValue(2.0);
    autoWahRate.setTextBoxIsEditable(false);
    autoWahRate.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 25);
    addAndMakeVisible(&autoWahRate);
    autoWahRate.setVisible(false);
    autoWahRate.addListener(this);

    attack.setSliderStyle(juce::Slider::Rotary);
    attack.setRange(0.001, 5.0, 0.001);
    attack.setSkewFactorFromMidPoint(0.1);
    attack.setTextBoxStyle(juce::Slider::TextBoxBelow, false, rotaryWidth * 1.1, rotaryWidth * 1.1);
    attack.setPopupDisplayEnabled(false, false, this);
    attack.setTextValueSuffix(" s attack");
    attack.setValue(audioProcessor.attack);
    addAndMakeVisible(&attack);
    attack.addListener(this);

    decay.setSliderStyle(juce::Slider::Rotary);
    decay.setRange(0.001, 5.0, 0.001);
    decay.setSkewFactorFromMidPoint(0.5);
    decay.setTextBoxStyle(juce::Slider::TextBoxBelow, false, rotaryWidth * 1.1, rotaryWidth * 1.1);
    decay.setPopupDisplayEnabled(false, false, this);
    decay.setTextValueSuffix(" s decay");
    decay.setValue(audioProcessor.decay);
    addAndMakeVisible(&decay);
    decay.addListener(this);

    sustain.setSliderStyle(juce::Slider::Rotary);
    sustain.setRange(0.0, 1.0, 0.01);
    sustain.setTextBoxStyle(juce::Slider::TextBoxBelow, false, rotaryWidth * 1.1, rotaryWidth * 1.1);
    sustain.setPopupDisplayEnabled(false, false, this);
    sustain.setTextValueSuffix(" sustain");
    sustain.setValue(audioProcessor.sustain);
    addAndMakeVisible(&sustain);
    sustain.addListener(this);

    release.setSliderStyle(juce::Slider::Rotary);
    release.setRange(0.001, 5.0, 0.001);
    release.setSkewFactorFromMidPoint(0.5);
    release.setTextBoxStyle(juce::Slider::TextBoxBelow, false, rotaryWidth * 1.1, rotaryWidth * 1.1);
    release.setPopupDisplayEnabled(false, false, this);
    release.setTextValueSuffix(" s release");
    release.setValue(audioProcessor.release);
    addAndMakeVisible(&release);
    release.addListener(this);
}

SynthAudioProcessorEditor::~SynthAudioProcessorEditor()
{
}

//==============================================================================
void SynthAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(15.0f));
}

void SynthAudioProcessorEditor::resized()
{
    int width = getWidth();
    int height = getHeight();
    int autoWahWidth = autoWahButton.getToggleState() ? autoWahExpansion : 0;

    gain.setBounds(margin, margin, width - margin * 2 - autoWahWidth, sliderHeight);
    pulseWidth.setBounds(margin, 2 * margin + sliderHeight, width - margin * 2 - autoWahWidth, sliderHeight);
    shape.setBounds(margin, 3 * margin + 2 * sliderHeight, (width - margin * 2 - autoWahWidth) * 2 / 3, comboBoxHeight);
    autoWahButton.setBounds(margin + (width - margin * 2 - autoWahWidth) * 2 / 3 + margin, 3 * margin + 2 * sliderHeight, (width - margin * 2 - autoWahWidth) / 6 - margin, comboBoxHeight);

    attack.setBounds(margin, 4 * margin + 2 * sliderHeight + comboBoxHeight, (width - margin * 5 - autoWahWidth) / 4, (width - margin * 5 - autoWahWidth) / 4);
    decay.setBounds(margin * 2 + ((width - margin * 5 - autoWahWidth) / 4), 4 * margin + 2 * sliderHeight + comboBoxHeight, (width - margin * 5 - autoWahWidth) / 4, (width - margin * 5 - autoWahWidth) / 4);
    sustain.setBounds(margin * 3 + (2 * (width - margin * 5 - autoWahWidth) / 4), 4 * margin + 2 * sliderHeight + comboBoxHeight, (width - margin * 5 - autoWahWidth) / 4, (width - margin * 5 - autoWahWidth) / 4);
    release.setBounds(margin * 4 + (3 * (width - margin * 5 - autoWahWidth) / 4), 4 * margin + 2 * sliderHeight + comboBoxHeight, (width - margin * 5 - autoWahWidth) / 4, (width - margin * 5 - autoWahWidth) / 4);

    if (autoWahButton.getToggleState())
    {
        int autoWahControlHeight = (height - 4 * margin) / 3;
        autoWahFrequency.setBounds(width - autoWahWidth + margin, margin, autoWahWidth - margin * 2, autoWahControlHeight);
        autoWahDepth.setBounds(width - autoWahWidth + margin, 2 * margin + autoWahControlHeight, autoWahWidth - margin * 2, autoWahControlHeight);
        autoWahRate.setBounds(width - autoWahWidth + margin, 3 * margin + 2 * autoWahControlHeight, autoWahWidth - margin * 2, autoWahControlHeight);
    }
}

void SynthAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &gain)
    {
        audioProcessor.gain = gain.getValue();
    }
    else if (slider == &pulseWidth)
    {
        audioProcessor.pulseWidth = pulseWidth.getValue();
    }
    else if (slider == &attack)
    {
        audioProcessor.attack = attack.getValue();
    }
    else if (slider == &decay)
    {
        audioProcessor.decay = decay.getValue();
    }
    else if (slider == &sustain)
    {
        audioProcessor.sustain = sustain.getValue();
    }
    else if (slider == &release)
    {
        audioProcessor.release = release.getValue();
    }
    else if (slider == &autoWahFrequency)
    {
        audioProcessor.autoWahFrequency = autoWahFrequency.getValue();
    }
    else if (slider == &autoWahDepth)
    {
        audioProcessor.autoWahDepth = autoWahDepth.getValue();
    }
    else if (slider == &autoWahRate)
    {
        audioProcessor.autoWahRate = autoWahRate.getValue();
    }
}

void SynthAudioProcessorEditor::comboBoxChanged(juce::ComboBox* comboBox)
{
    if (comboBox == &shape)
    {
        audioProcessor.waveType = (WaveType)shape.getSelectedId();
    }
}

void SynthAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (button == &autoWahButton)
    {
        bool isAutoWahEnabled = autoWahButton.getToggleState();
        audioProcessor.autoWah = isAutoWahEnabled;
        autoWahFrequency.setVisible(isAutoWahEnabled);
        autoWahDepth.setVisible(isAutoWahEnabled);
        autoWahRate.setVisible(isAutoWahEnabled);

        if (isAutoWahEnabled)
        {
            setSize(getWidth() + autoWahExpansion, getHeight());
        }
        else
        {
            setSize(getWidth() - autoWahExpansion, getHeight());
        }
    }
}