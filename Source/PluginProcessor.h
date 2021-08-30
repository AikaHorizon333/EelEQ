/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================


enum Slope {
    
    Slope_12,
    Slope_24,
    Slope_36,
    Slope_48
    
};


struct ChainSettings {
  
    float peakFreq {0}, peakGainInDecibels{0}, peakQuality{0};
    float lowCutFreq{0}, highCutFreq{0};
    Slope  lowCutSlope{ Slope::Slope_12 }, highCutSlope{ Slope::Slope_12 };

};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

using Filter = juce::dsp::IIR::Filter<float>;

using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;

using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>; // How the signal moves throguh the plugin


enum ChainPositions {
    LowCut,
    Peak,
    HighCut
};

using Coefficients = Filter::CoefficientsPtr;
void UpdateCoefficients (Coefficients& old, const Coefficients& replacements);

Coefficients makesPeakFilter();


//==============================================================================
/**
*/
class EelEQAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    EelEQAudioProcessor();
    ~EelEQAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //==============================================================================
    
    
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    juce::AudioProcessorValueTreeState apvts {*this, nullptr,"Parameters", createParameterLayout() };
    
    //====
    

private:
    
    
    MonoChain leftChain, rightChain; // Stereo capabilities.
    
    // Declaramos una función auxiliar. 
    void UpdatePeakFilter(const ChainSettings& chainSettings);
    
    
    
    
    template<int Index,typename ChainType, typename CoefficientType>
    void updateSlopes(ChainType& chain, CoefficientType& cutCoeffients)
        {
           
            // Actualizar los coeficientes, desactivar el bypass de la cadena.
            *chain.template get<Index>().coefficients = *cutCoeffients[Index];
            chain.template setBypassed<Index>(false);
            
        }
    
    
    template<typename ChainType, typename CoefficientType>
    void UpdateCutFilter(ChainType &chain,
                         const CoefficientType& cutCoefficients,
                         const Slope& slope)
    
    {
        
        //Filter Channel
        
        chain.template setBypassed<0>(true);
        chain.template setBypassed<1>(true);
        chain.template setBypassed<2>(true);
        chain.template setBypassed<3>(true);
        
        
        switch (slope)
        {
            
            case Slope_48:
            {
                updateSlopes<3>(chain, cutCoefficients);
            }
            
            case Slope_36:
            {
                updateSlopes<2>(chain, cutCoefficients);
            }
                
            case Slope_24:
            {
                updateSlopes<1>(chain, cutCoefficients);
            }
                
            case Slope_12:
            {
                updateSlopes<0>(chain, cutCoefficients);
            }
                
        }
        
        
    }
    
    void UpdateLowCut(const ChainSettings& chainSettings);
    void UpdateHighCut(const ChainSettings& chainSettings);
    
    void UpdateFilters();
    
    
    
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EelEQAudioProcessor)
};
