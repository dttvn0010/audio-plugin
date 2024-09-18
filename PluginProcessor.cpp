/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <math.h>

//==============================================================================
SimplePluginAudioProcessor::SimplePluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
    , state (*this, nullptr, "STATE", {
        std::make_unique<juce::AudioParameterFloat> ("gain",    "Gain",     0.0f, 2.0f, 1.0f),
        std::make_unique<juce::AudioParameterFloat> ("bass",    "Bass",    -1.0f, 1.0f, 0.0f),
        std::make_unique<juce::AudioParameterFloat> ("trebble",  "Trebble", -1.0f, 1.0f, 0.0f)
    })
{
}

SimplePluginAudioProcessor::~SimplePluginAudioProcessor()
{
}

//==============================================================================
const juce::String SimplePluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimplePluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimplePluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimplePluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimplePluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimplePluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimplePluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimplePluginAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimplePluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimplePluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimplePluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void SimplePluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimplePluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void bwlpf(const float* inp, float* out, int n_samples)
{
    double M_PI = 3.141592653589793;
    int n = 4; // 8/2
    double s = 44100;
    double f = 4000;
    
    double a = tan(M_PI * f / s);
    double a2 = a * a;

    double A[4];
    double d1[4];
    double d2[4];
    double w0[4]{ 0,0,0,0 };
    double w1[4]{ 0,0,0,0 };
    double w2[4]{ 0,0,0,0 };

    double r;
    for (int i = 0; i < n; i++)
    {
        r = sin(M_PI * (2.0 * i + 1.0) / (4.0 * n));
        s = a2 + 2.0 * a * r + 1.0;
        A[i] = a2 / s;
        d1[i] = 2.0 * (1 - a2) / s;
        d2[i] = -(a2 - 2.0 * a * r + 1.0) / s;
    }

    for (int k = 0; k < n_samples; k++)
    {
        double x = inp[k];
        for (int i = 0; i < n; ++i) {
            w0[i] = d1[i] * w1[i] + d2[i] * w2[i] + x;
            x = A[i] * (w0[i] + 2.0 * w1[i] + w2[i]);
            w2[i] = w1[i];
            w1[i] = w0[i];
        }
        out[k] = float(x);
    }
}

void bwhpf(const float* inp, float* out, int n_samples)
{
    double M_PI = 3.141592653589793;
    int n = 4; // 8/2
    double s = 44100;
    double f = 8000;

    double a = tan(M_PI * f / s);
    double a2 = a * a;

    double A[4];
    double d1[4];
    double d2[4];
    double w0[4]{ 0,0,0,0 };
    double w1[4]{ 0,0,0,0 };
    double w2[4]{ 0,0,0,0 };

    double r;
    for (int i = 0; i < n; ++i) 
    {
        r = sin(M_PI * (2.0 * i + 1.0) / (4.0 * n));
        s = a2 + 2.0 * a * r + 1.0;
        A[i] = 1.0 / s;
        d1[i] = 2.0 * (1 - a2) / s;
        d2[i] = -(a2 - 2.0 * a * r + 1.0) / s;
    }

    for (int k = 0; k < n_samples; k++)
    {
        double x = inp[k];
        for (int i = 0; i < n; ++i) 
        {
            w0[i] = d1[i] * w1[i] + d2[i] * w2[i] + x;
            x = A[i] * (w0[i] - 2.0 * w1[i] + w2[i]);
            w2[i] = w1[i];
            w1[i] = w0[i];
        }
        out[k] = float(x);
    }
}

void SimplePluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.

    float gain = 2 * state.getParameter ("gain")->getValue();
    float bass = state.getParameter ("bass")->getValue();
    float trebble = state.getParameter ("trebble")->getValue();
   
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer (channel);
        
        int n = buffer.getNumSamples();
        float* tmp1 = (float*) calloc(n, sizeof(float));
        float* tmp2 = (float*) calloc(n, sizeof(float));
        if (tmp1 && tmp2)
        {
            if (channelData[0] > 0)
            {
                int x = 1;
            }
            bwlpf(channelData, tmp1, n);
            bwhpf(channelData, tmp2, n);

            for (int i = 0; i < n; ++i)
            {
                channelData[i] = gain * (channelData[i] +  (bass - 0.5f) * tmp1[i] + (trebble - 0.5f) * tmp2[i]);
            }
        }
        if (tmp1) free(tmp1);
        if (tmp2) free(tmp2);
    }
}

//==============================================================================
bool SimplePluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimplePluginAudioProcessor::createEditor()
{
    return new SimplePluginAudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void SimplePluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    if (auto xmlState = state.copyState().createXml())
        copyXmlToBinary (*xmlState, destData);
}

void SimplePluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    if (auto xmlState = getXmlFromBinary (data, sizeInBytes))
        state.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimplePluginAudioProcessor();
}
