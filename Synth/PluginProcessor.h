/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

enum WaveType
{
    Sine = 1,
    Sawtooth = 2,
    Square = 3,
    Triangle = 4
};

class Voice
{
public:
    Voice::Voice() : angle(0.0), angleDelta(0.0), currentVolume(0.0), currentPhaseMultiplier(0.0), phase(0), startTimestamp(0), endTimestamp(0) {}
	~Voice() {}
	void setFrequency(double newFrequency) { frequency = newFrequency; updateAngleDelta(); }
	void setVelocity(double newVelocity) { velocity = newVelocity; updateCurrentVolume(); }
	void noteOn() { startTimestamp = juce::Time::getMillisecondCounter(); phase = 1; angle = 0.0; }
	void noteOff() { endTimestamp = juce::Time::getMillisecondCounter(); phase = 4; }

	void setSampleRate(double sampleRate);
	void setGlobalParameters(double gain, double pulseWidth, WaveType waveType, double attack, double decay, double sustain, double release);
	double getFrequency() const { return frequency; }
	bool isPlaying() const { return phase != 0; }

	void processBlock(juce::AudioBuffer<float>& buffer);

private:
	void updateAngleDelta();
	void updateCurrentVolume();
	double angle = 0.0, angleDelta = 0.0, frequency = 440.0;
	double velocity = 0;	//0-127
	double currentPhaseMultiplier = 0;	//0-1
	double currentVolume = 0;	//0-1
	int phase = 0; // 1 = attack, 2 = decay, 3 = sustain, 4 = release, 0 = off
	unsigned int startTimestamp = 0;
	unsigned int endTimestamp = 0;

	double sampleRate = 0.0;
	double gain = 0;	//0-1
	double pulseWidth = 0.5;	//0-1
	WaveType waveType = Sine;
	double attack = 0.02;	//seconds
	double decay = 0.04;	//seconds
	double sustain = 0.7;	//0-1
	double release = 0.03;	//seconds
};


//==============================================================================
/**
*/
class SynthAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    SynthAudioProcessor();
    ~SynthAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;


    //==============================================================================
    void updateAutoWahFilter(double currentTimeInSeconds);
    void updateAutoWahFilterWithPhase();

    juce::IIRFilter autoWahFilter;
    double autoWahFrequency = 700.0;
    double autoWahDepth = 0.8;
    double autoWahRate = 2.0;
    bool autoWah = false;

    double lfoPhase = 0.0;
    double lfoPhaseIncrement = 0.0;

    int maxVoices = 16;
    juce::OwnedArray<Voice> voices;
    double gain = 0.2512;
    double pulseWidth = 0.5;
    WaveType waveType = Sine;
    double attack = 0.02;
    double decay = 0.04;
    double sustain = 0.7;
    double release = 0.03;

private:
    double currentSampleRate = 0.0;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthAudioProcessor)
};

//==============================================================================