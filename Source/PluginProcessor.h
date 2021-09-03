/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <array>

//==============================================================================
//FFT implementation 3: Fifo type templeate...

template<typename T>
struct Fifo
{
    void prepare (int numChannels, int numSamples){
        
        //Fixing the BlockType and Vector bug...
        static_assert(std::is_same_v<T, juce::AudioBuffer<float>>,
                      "prepare(numChannels, numSamples) should only be used when the Fifo is holding juce::AudioBuffer<float>" );
        
        for (auto& buffer : buffers)
        {
            
            buffer.setSize(numChannels,
                           numSamples,
                           false,       //Clear everything?
                           true,        //including the extra space?
                           true);       //avoid realocating if you can?
            
            buffer.clear();
            
        }
    }
    
    void prepare(size_t numElements)
    {
        static_assert(std::is_same_v<T, std::vector<float>>,
                      "prepare(numElements) should only be used when the Fifo is holding std::vector<float>" );
        
        for(auto& buffer : buffers)
        {
            
            buffer.clear();
            buffer.resize(numElements, 0);
            
        }
        
    }
    
    bool push(const T& t)
    {
        auto write = fifo.write(1);
       
        if(write.blockSize1>0)
        {
            buffers[write.startIndex1] = t;
            return true;
        }
        
        return false;
    }
    
    bool pull (T& t)
    {
        auto read = fifo.read(1);
        if (read.blockSize1>0)
        {
            t = buffers[read.startIndex1];
            return true;
            
        }
        
        return false;
    }
    
    int getNumAvailableForReading() const
    {
        
        return fifo.getNumReady();
        
    }
    
private:
    static constexpr int Capacity = 30;
    std::array<T, Capacity> buffers;
    juce::AbstractFifo fifo {Capacity};
    
};
//==============================================================================
// FFT implementation 1: Channel

enum Channel{
    
    Right, //effectivly 0
    Left   //effectinly 1
    
};


//==============================================================================
//FFT implementation 2: SingleChannelSampleFifo

template<typename BlockType>
struct SingleChannelSampleFifo
{
    SingleChannelSampleFifo(Channel ch) : channelToUse(ch)
    {
        prepared.set(false);
    }
    
    void update (const BlockType& buffer){
        
        jassert(prepared.get());
        jassert(buffer.getNumChannels() > channelToUse );
        auto* channelPtr = buffer.getReadPointer(channelToUse);
        
        for (int i = 0; i < buffer.getNumSamples(); ++i){
            
            pushNextSampleIntoFifo(channelPtr[i]);
            
        }
    }
    
    void prepare(int bufferSize){
        
        prepared.set(false);
        size.set(bufferSize);
        
        bufferToFill.setSize(1,          //Canal
                             bufferSize, //num of samples
                             false,      //keep existing content
                             true,       //clear extra space
                             true);      //avoid reallocating
        
        audioBufferFifo.prepare(1, bufferSize);
        fifoIndex = 0;
        prepared.set(true);
    }
    //===========
    
    int getNumCompleteBuffersAvailable() const { return audioBufferFifo.getNumAvailableForReading();}
    bool isPrepared() const {return prepared.get();}
    int getSize() const {return size.get();}
    
    //===========
    bool getAudioBuffer(BlockType& buf){return audioBufferFifo.pull(buf);}
    
private:
    Channel channelToUse;
    int fifoIndex = 0;
    Fifo<BlockType> audioBufferFifo;
    BlockType bufferToFill;
    juce::Atomic<bool> prepared = false;
    juce::Atomic<int> size = 0;
    
    
    void pushNextSampleIntoFifo(float sample){
        
        if (fifoIndex == bufferToFill.getNumSamples()){
            
            auto ok = audioBufferFifo.push(bufferToFill);
            juce::ignoreUnused(ok);
            fifoIndex = 0;
            
        }
    
        bufferToFill.setSample(0,fifoIndex,sample);
        ++fifoIndex;
    
    }
};

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

Coefficients makePeakFilter(const ChainSettings& chainSettings, double sampleRate);


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




inline auto makeLowCutFilter(const ChainSettings& chainSettings, double sampleRate){
    
    return juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq, sampleRate, 2*(chainSettings.lowCutSlope + 1));

    
}

inline auto makeHighCutFilter (const ChainSettings& chainSettings, double sampleRate){
    
    
    return juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.highCutFreq, sampleRate, 2*(chainSettings.highCutSlope +1));
    
}





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
    
    //==============================================================================

    // Crear las instancias de los FIFO y el namespace para poder declarar pointers de forma facil.
    
    using BlockType = juce::AudioBuffer<float>;
    SingleChannelSampleFifo<BlockType> leftChannelFifo {Channel::Left};
    SingleChannelSampleFifo<BlockType> rightChannelFifo{Channel::Right};
    
private:
    
    
    MonoChain leftChain, rightChain; // Stereo capabilities.
    
    // Declaramos una función auxiliar. 
    void UpdatePeakFilter(const ChainSettings& chainSettings);
    void UpdateLowCut(const ChainSettings& chainSettings);
    void UpdateHighCut(const ChainSettings& chainSettings);
    
    void UpdateFilters();
    
    //Creamos un Oscilador para calibrar la FFT
    
    juce::dsp::Oscillator<float> osc;
    
    
    
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EelEQAudioProcessor)
};
