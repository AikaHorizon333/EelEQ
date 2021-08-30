/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================


struct CustomRotatorySlider: juce::Slider {
    
    CustomRotatorySlider(): juce::Slider (juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                                          juce::Slider::TextEntryBoxPosition::NoTextBox){
        
        
    }
        
};





//==============================================================================
/**
*/
class EelEQAudioProcessorEditor  : public juce::AudioProcessorEditor,
juce::AudioProcessorParameter::Listener,
juce::Timer
{
public:
    EelEQAudioProcessorEditor (EelEQAudioProcessor&);
    ~EelEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    
    // Listener and Timer
    void parameterValueChanged (int parameterIndex, float newValue) override;
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override{ };
    void timerCallback() override;
    
    
    
    

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    EelEQAudioProcessor& audioProcessor;
    juce::Atomic<bool> parametersChanged {false};
    
    // Declarar los sliders
    CustomRotatorySlider peakFreqSlider, peakGainSlider, peakQualitySlider,
    lowcutFreqSlider, highcutFreqSlider,
    lowcutSlopeSlider, highcutSlopeSlider;
    
    // Funcion Auxiliar para modificar los sliders
    
    std::vector<juce::Component*> getComp();
    
    // Namespace para obtener los parametros
    
    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;
    
    
    Attachment peakFreqSliderAttachment, peakGainSliderAttachment, peakQualitySliderAttachment,
    lowcutFreqSliderAttachment, highcutFreqSliderAttachment,
    lowcutSlopeSliderAttachment, highcutSlopeSliderAttachment;
    
    
    MonoChain monoChain;
    
    
    
    
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EelEQAudioProcessorEditor);
};
