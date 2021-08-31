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

//Separar la Curva de respuesta del Editor.
//Para esto hay que heredar las mismas clases que estamos usando en el editor: APEditor, Listener y Timer.

struct ResponseCurveComponent :
juce::Component,
juce::AudioProcessorParameter::Listener,
juce::Timer
{
    //Constructor y destructor
    ResponseCurveComponent(EelEQAudioProcessor&);
    ~ResponseCurveComponent();
    EelEQAudioProcessor& audioProcessor;
    
    // Listener and Timer Callbacks
    void parameterValueChanged (int parameterIndex, float newValue) override;
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override{ };
    void timerCallback() override;
    juce::Atomic<bool> parametersChanged {false};
    
    
    //Herencia del editor.
    void paint (juce::Graphics& g) override;
    
    //MonoChain
    MonoChain monoChain;
    
};
 











//==============================================================================
/**
*/
class EelEQAudioProcessorEditor  : public juce::AudioProcessorEditor

{
public:
    EelEQAudioProcessorEditor (EelEQAudioProcessor&);
    ~EelEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    
    

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    EelEQAudioProcessor& audioProcessor;
    
    // Declarar los sliders
    CustomRotatorySlider peakFreqSlider, peakGainSlider, peakQualitySlider,
    lowcutFreqSlider, highcutFreqSlider,
    lowcutSlopeSlider, highcutSlopeSlider;
    
    //ResponseCurveComponent
    ResponseCurveComponent responseCurveComponent;
    
    
    // Funcion Auxiliar para modificar los sliders
    
    std::vector<juce::Component*> getComp();
    
    // Namespace para obtener los parametros
    
    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;
    
    
    Attachment peakFreqSliderAttachment, peakGainSliderAttachment, peakQualitySliderAttachment,
    lowcutFreqSliderAttachment, highcutFreqSliderAttachment,
    lowcutSlopeSliderAttachment, highcutSlopeSliderAttachment;
    
    
    
    
    
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EelEQAudioProcessorEditor);
};
