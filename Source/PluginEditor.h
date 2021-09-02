/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================

//Look and Feel de RotarySliderWithLabels

struct LookAndFeel: juce::LookAndFeel_V4{
    
    void drawRotarySlider (juce::Graphics&,
                                   int x, int y, int width, int height,
                                   float sliderPosProportional,
                                   float rotaryStartAngle,
                                   float rotaryEndAngle,
                           juce::Slider&) override;
   
};

struct RotarySliderWithLabels: juce::Slider {
    
    
    //constructor y destructor
    RotarySliderWithLabels(juce::RangedAudioParameter &rap, const juce::String unitSuffix): juce::Slider (juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
    juce::Slider::TextEntryBoxPosition::NoTextBox),
    param(&rap),
    suffix(unitSuffix)
                                            
        {
            setLookAndFeel(&lnf);
        }
    
    ~RotarySliderWithLabels(){
        
        setLookAndFeel(nullptr);
        
    }
    
    // Etiquetas de valores minimos y máximos...
    
    struct LabelPos {
        
        float pos; //normalized possition
        juce::String label; //label name ex:  20 Hz - 20kHz.
        
    };
    
    
    juce::Array<LabelPos> labels;
    
    
    //Instancias para el LookAndFeel
    juce::RangedAudioParameter* param;
    juce::String suffix;
    LookAndFeel lnf;
    
    
    //Metodo de paint
    void paint(juce::Graphics &g)override;
    
    //TextBox
    juce::Rectangle<int> getSliderBounds() const;
    int getTextHeight() const {return 14;}
    juce::String getDisplayString() const;
    
    
    
    
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
    void resized()override;
    
    //BG Image for frequency Plot
    juce::Image background;
    
    juce::Rectangle<int> getRenderArea();
    juce::Rectangle<int> getAnalysisArea();
    
    //MonoChain
    MonoChain monoChain;
    
    void UpdateChain();
    
    //FFT
    
    
    
    
    
    
    
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
    RotarySliderWithLabels peakFreqSlider, peakGainSlider, peakQualitySlider,
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
