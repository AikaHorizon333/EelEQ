/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
EelEQAudioProcessorEditor::EelEQAudioProcessorEditor (EelEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    peakFreqSliderAttachment(audioProcessor.apvts, "Peak Freq", peakFreqSlider),
    peakGainSliderAttachment(audioProcessor.apvts, "Peak Gain", peakGainSlider),
    peakQualitySliderAttachment(audioProcessor.apvts, "Quality", peakQualitySlider),
    lowcutFreqSliderAttachment(audioProcessor.apvts, "LowCut Freq", lowcutFreqSlider),
    highcutFreqSliderAttachment(audioProcessor.apvts, "HighCut Freq",  highcutFreqSlider),
    lowcutSlopeSliderAttachment(audioProcessor.apvts, "LowCut Slope", lowcutSlopeSlider),
    highcutSlopeSliderAttachment(audioProcessor.apvts, "HighCut Slope", highcutSlopeSlider)




{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    for(auto* comp : getComp())
    {
        
        addAndMakeVisible(comp);
        
    }
    
    // Add listeer
    
    const auto& params = audioProcessor.getParameters();
    for(auto param: params){
        param->addListener(this);
    }
    
    // start the timer (very importante)
    startTimerHz(60);
    
    setSize (600, 400);
}

EelEQAudioProcessorEditor::~EelEQAudioProcessorEditor()
{
    
    //Remove listener on exit...
    const auto& params = audioProcessor.getParameters();
    for(auto param: params){
        param->removeListener(this);
    }
    
}

//==============================================================================
void EelEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::black);
    
    //Drawing the response curve...
    
    //We need to get the bounds and the width of the response area.
    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight()*0.33); // esto regresa la zona que le corresponde la la curva.
    
    auto w = responseArea.getWidth();
    
    auto& lowcut = monoChain.get<ChainPositions::LowCut>();
    auto& peak = monoChain.get<ChainPositions::Peak>();
    auto& highcut = monoChain.get<ChainPositions::HighCut>();
    
    
    auto sampleRate = audioProcessor.getSampleRate();
    std::vector<double> mags;
    mags.resize(w);
    
    for(int i = 0; i<w; ++i){
        
        double mag = 1.f;
        //mapping the frequency to human hearing range.
        
        auto freq = mapToLog10<double>(double(i)/double(w), 20, 20000);
        
        //peak
        if(! monoChain.isBypassed<ChainPositions::Peak>()){
            mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        
        //lowcuts
        if (!lowcut.isBypassed<0>()){
            mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!lowcut.isBypassed<1>()){
            mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!lowcut.isBypassed<2>()){
            mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!lowcut.isBypassed<3>()){
            mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        
        //highcut
        if (!highcut.isBypassed<0>()){
            mag *= highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!highcut.isBypassed<1>()){
            mag *= highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!highcut.isBypassed<2>()){
            mag *= highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!highcut.isBypassed<3>()){
            mag *= highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        
        
        mags[i] = Decibels::gainToDecibels(mag);
        
    }
    
    Path responseCurve;

    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    auto map = [outputMin, outputMax](double input){
        
        return jmap(input, -24.0, 24.0, outputMin, outputMax);
        
    };
    
    //this starts the mapping
    responseCurve.startNewSubPath(responseArea.getX(), map(mags.front())); // this will run the first value of the jmap.
    
    //this checks the values and maps them into the response curve.
    for( size_t i =1; i < mags.size(); ++i){
        
        responseCurve.lineTo(responseArea.getX()+i, map(mags[i]));
    }
    
    // Drawing the Response curve
    g.setColour(Colours::orange);
    g.drawRoundedRectangle(responseArea.toFloat(), 4.f, 1.5f);
    
    // Drawing the path
    g.setColour(Colours::white);
    g.strokePath(responseCurve, PathStrokeType(2.f));
    
    
    
    
    
    
    
    
}

void EelEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight()*0.33);
    
    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth()*0.33);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth()*0.5);
    
    lowcutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight()*0.5));
    lowcutSlopeSlider.setBounds(lowCutArea);
    
    highcutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight()*0.5));
    highcutSlopeSlider.setBounds(highCutArea);
    
    
    peakFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight()*0.33));
    peakGainSlider.setBounds(bounds.removeFromTop(bounds.getHeight()*0.5));
    peakQualitySlider.setBounds(bounds);
    
}

void EelEQAudioProcessorEditor::parameterValueChanged(int parameterIndex, float newValue){
    
    // Set the atomic flag
    parametersChanged.set(true);
    
}

void EelEQAudioProcessorEditor::timerCallback(){
    
    // Solo va a actualizar si se realizó algun cambio en el parametro
    if(parametersChanged.compareAndSetBool(false, true))
    {
        // Actualizar el monoChain
        
        DBG("Params Change");
        
        auto chainSettings = getChainSettings(audioProcessor.apvts);
        auto peakCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate());
        UpdateCoefficients(monoChain.get<ChainPositions::Peak>().coefficients,peakCoefficients);
        
        auto lowcutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
        auto highcutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());
        
     
        // Redibujar la curva
        repaint();
        
        
    }
    
    
}


std::vector<juce::Component*> EelEQAudioProcessorEditor::getComp(){
    
    return
    {
        &peakFreqSlider,
        &peakGainSlider,
        &peakQualitySlider,
        &lowcutFreqSlider,
        &highcutFreqSlider,
        &lowcutSlopeSlider,
        &highcutSlopeSlider
        
        
    };
    
}


