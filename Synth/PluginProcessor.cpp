#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SynthAudioProcessor::SynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif 

    )
#endif
{
    for (int i = 0; i < maxVoices; ++i)
        voices.add(new Voice());
}

SynthAudioProcessor::~SynthAudioProcessor()
{
}

//==============================================================================
const juce::String SynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SynthAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool SynthAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool SynthAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double SynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SynthAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int SynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SynthAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String SynthAudioProcessor::getProgramName(int index)
{
    return {};
}

void SynthAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void SynthAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    for (auto* voice : voices)
    {
        voice->setSampleRate(sampleRate);
        voice->setGlobalParameters(gain, pulseWidth, waveType, attack, decay, sustain, release);
		voice->noteOn();
		voice->noteOff();
    }

    autoWahFilter.reset();
    lfoPhase = 0.0;
    lfoPhaseIncrement = autoWahRate / sampleRate;
    updateAutoWahFilter(0.0);
}


void SynthAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SynthAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void SynthAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    buffer.clear();

    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        if (message.isNoteOn())
        {
            for (auto* voice : voices)
            {
                if (!voice->isPlaying())
                {
                    voice->setFrequency(juce::MidiMessage::getMidiNoteInHertz(message.getNoteNumber()));
                    voice->setVelocity(message.getVelocity());
                    voice->noteOn();
                    break;
                }
                else if (voice->isPlaying() && voice->getFrequency() == juce::MidiMessage::getMidiNoteInHertz(message.getNoteNumber()))
                {
                    voice->noteOff();
                    voice->setFrequency(juce::MidiMessage::getMidiNoteInHertz(message.getNoteNumber()));
                    voice->setVelocity(message.getVelocity());
                    voice->noteOn();
                    break;
                }
            }
        }
        else if (message.isNoteOff())
        {
            for (auto* voice : voices)
            {
                if (voice->isPlaying() && voice->getFrequency() == juce::MidiMessage::getMidiNoteInHertz(message.getNoteNumber()))
                {
                    voice->noteOff();
                    break;
                }
            }
        }
    }

    for (auto* voice : voices)
    {
        voice->setGlobalParameters(gain, pulseWidth, waveType, attack, decay, sustain, release);
        if (voice->isPlaying())
            voice->processBlock(buffer);
    }

    if (autoWah)
    {
        if (auto* playHead = getPlayHead())
        {
            juce::AudioPlayHead::CurrentPositionInfo positionInfo;
            if (playHead->getCurrentPosition(positionInfo))
            {
                updateAutoWahFilter(positionInfo.timeInSeconds);
            }
            else
            {
                updateAutoWahFilterWithPhase();
            }
        }
        else
        {
            updateAutoWahFilterWithPhase();
        }

        auto* firstChannelData = buffer.getWritePointer(0);
        
        for (int channel = 1; channel < buffer.getNumChannels(); ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            autoWahFilter.processSamples(channelData, buffer.getNumSamples());
            //channelData = firstChannelData;
        }
    }
}



void SynthAudioProcessor::updateAutoWahFilter(double currentTimeInSeconds)
{
    if (autoWah)
    {
        double lfo = 0.5 * (1.0 + std::sin(2.0 * juce::MathConstants<double>::pi * autoWahRate * currentTimeInSeconds));
        double cutoff = autoWahFrequency + autoWahDepth * lfo * autoWahFrequency;
        autoWahFilter.setCoefficients(juce::IIRCoefficients::makeBandPass(currentSampleRate, cutoff, 1.0));
    }
}

void SynthAudioProcessor::updateAutoWahFilterWithPhase()
{
    if (autoWah)
    {
        double lfo = 0.5 * (1.0 + std::sin(2.0 * juce::MathConstants<double>::pi * lfoPhase));
        double cutoff = autoWahFrequency + autoWahDepth * lfo * autoWahFrequency;
        autoWahFilter.setCoefficients(juce::IIRCoefficients::makeBandPass(currentSampleRate, cutoff, 1.0));

        lfoPhase += lfoPhaseIncrement;
        if (lfoPhase >= 1.0)
            lfoPhase -= 1.0;
    }
}


//==============================================================================
bool SynthAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SynthAudioProcessor::createEditor()
{
    return new SynthAudioProcessorEditor(*this);
}

//==============================================================================
void SynthAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    destData.append("", 1);
    destData.append(&gain, sizeof(gain));
    destData.append(&pulseWidth, sizeof(pulseWidth));
    destData.append(&waveType, sizeof(waveType));
    destData.append(&attack, sizeof(attack));
    destData.append(&decay, sizeof(decay));
    destData.append(&sustain, sizeof(sustain));
    destData.append(&release, sizeof(release));
    destData.append(&autoWahFrequency, sizeof(autoWahFrequency));
    destData.append(&autoWahDepth, sizeof(autoWahDepth));
    destData.append(&autoWahRate, sizeof(autoWahRate));
    destData.append(&autoWah, sizeof(autoWah));
}

void SynthAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    const char* d = static_cast<const char*>(data);
    d += 1;
    gain = *reinterpret_cast<const double*>(d);
    d += sizeof(double);
    pulseWidth = *reinterpret_cast<const double*>(d);
    d += sizeof(double);
    waveType = *reinterpret_cast<const WaveType*>(d);
    d += sizeof(WaveType);
    attack = *reinterpret_cast<const double*>(d);
    d += sizeof(double);
    decay = *reinterpret_cast<const double*>(d);
    d += sizeof(double);
    sustain = *reinterpret_cast<const double*>(d);
    d += sizeof(double);
    release = *reinterpret_cast<const double*>(d);
    d += sizeof(double);
    autoWahFrequency = *reinterpret_cast<const double*>(d);
    d += sizeof(double);
    autoWahDepth = *reinterpret_cast<const double*>(d);
    d += sizeof(double);
    autoWahRate = *reinterpret_cast<const double*>(d);
    d += sizeof(double);
    autoWah = *reinterpret_cast<const bool*>(d);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SynthAudioProcessor();
}



void Voice::setSampleRate(double sampleRate) {
    this->sampleRate = sampleRate;
    updateAngleDelta();
}

void Voice::setGlobalParameters(double gain, double pulseWidth, WaveType waveType, double attack, double decay, double sustain, double release) {
    this->gain = gain;
    this->pulseWidth = pulseWidth;
    this->waveType = waveType;
    this->attack = attack;
    this->decay = decay;
    this->sustain = sustain;
    this->release = release;
    //updateAngleDelta();
    updateCurrentVolume();
}

void Voice::processBlock(juce::AudioBuffer<float>& buffer) {
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        double sample = 0.0;
        switch (waveType) {
        case Sine:
            sample = std::sin(angle);
            break;
        case Sawtooth:
            sample = (angle / juce::MathConstants<double>::pi);
            sample = sample < 1.0 ? sample : sample - 2.0;
            break;
        case Square:
            sample = angle < juce::MathConstants<double>::twoPi * pulseWidth ? 1.0 : -1.0;
            break;
        case Triangle:
            sample = 2 * angle / juce::MathConstants<double>::pi;
            if (sample > 1.0)
                if (sample < 3.0)
                    sample = 3.0 - sample;
                else
                    sample = sample - 4.0;
            break;
        }
        angle += angleDelta;
        updateCurrentVolume();
        if (angle >= juce::MathConstants<double>::twoPi)
            angle -= juce::MathConstants<double>::twoPi;

        auto* firstChannelData = buffer.getWritePointer(0);
        firstChannelData[i] += static_cast<float>(sample * currentVolume);
        for (int channel = 1; channel < buffer.getNumChannels(); ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            channelData[i] += firstChannelData[i];
        }
    }
}

void Voice::updateAngleDelta() {
	angle = 0.0;
    auto cyclesPerSample = frequency / sampleRate;
    angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
}

void Voice::updateCurrentVolume() {
    switch (phase) {
    case 1:
        currentPhaseMultiplier = (juce::Time::getMillisecondCounter() - startTimestamp) / (attack * 1000);
        if (currentPhaseMultiplier >= 1) {
            currentPhaseMultiplier = 1;
            phase = 2;
            startTimestamp = juce::Time::getMillisecondCounter();
        }
        break;
    case 2:
    {
        double percent = (juce::Time::getMillisecondCounter() - startTimestamp) / (decay * 1000);
        //currentPhaseMultiplier = 1 - (percent * (1 - sustain));
        currentPhaseMultiplier = (1 - percent) * (1 - sustain) + sustain;
        if (currentPhaseMultiplier <= sustain) {
            currentPhaseMultiplier = sustain;
            phase = 3;
        }
    }
    break;
    case 3:
        currentPhaseMultiplier = sustain;
        break;
    case 4:
    {
        double percent = (juce::Time::getMillisecondCounter() - endTimestamp) / (release * 1000);
        currentPhaseMultiplier = (1 - percent) * sustain;
        if (currentPhaseMultiplier <= 0) {
            currentPhaseMultiplier = 0;
            phase = 0;
			angle = 0.0;
        }
    }
    break;
    }

    currentVolume = gain * currentPhaseMultiplier * velocity / 127;
}
