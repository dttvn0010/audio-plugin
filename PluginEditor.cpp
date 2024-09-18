/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimplePluginAudioProcessorEditor::SimplePluginAudioProcessorEditor (SimplePluginAudioProcessor& p)
    : juce::AudioProcessorEditor (&p), audioProcessor (p),
      gainAttachment    (p.state, "gain",       gainSlider),
      bassAttachment    (p.state, "bass",       bassSlider),
      trebbleAttachment (p.state, "trebble",    trebbleSlider)
{
    gainSlider.setSliderStyle (juce::Slider::SliderStyle::LinearVertical);
    bassSlider.setSliderStyle (juce::Slider::SliderStyle::LinearVertical);
    trebbleSlider.setSliderStyle (juce::Slider::SliderStyle::LinearVertical);

    /*
    for (auto* slider : { &gainSlider, &bassSlider, &trebbleSlider })
    {
        slider->setTextBoxStyle (juce::Slider::TextBoxBelow, true, 200, 30);
        //addAndMakeVisible (slider);
    }*/

    //gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 200, 30);
    gainSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    addAndMakeVisible(&gainSlider);

    bassSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    addAndMakeVisible(&bassSlider);

    trebbleSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    addAndMakeVisible(&trebbleSlider);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
}

SimplePluginAudioProcessorEditor::~SimplePluginAudioProcessorEditor()
{
}

//==============================================================================
void SimplePluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff121212));
    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
    g.drawFittedText("Volume", juce::Rectangle<int>(0, getHeight() - 30, getWidth()/3, 20), juce::Justification::centred, 1);
    g.drawFittedText("Bass", juce::Rectangle<int>(getWidth() / 3, getHeight() - 30, getWidth() / 3, 20), juce::Justification::centred, 1);
    g.drawFittedText("Trebble", juce::Rectangle<int>(2 * getWidth() / 3, getHeight() - 30, getWidth() / 3, 20), juce::Justification::centred, 1);
}

void SimplePluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    juce::Rectangle<int> gainBounds(0, 10, getWidth() / 3, getHeight() - 40);
    gainSlider.setBounds (gainBounds);

    juce::Rectangle<int> bassBounds(getWidth() / 3, 10, getWidth() / 3, getHeight() - 40);
    bassSlider.setBounds(bassBounds);

    juce::Rectangle<int> trebbleBounds(2 *getWidth()/3, 10, getWidth() / 3, getHeight() - 40);
    trebbleSlider.setBounds(trebbleBounds);
}
