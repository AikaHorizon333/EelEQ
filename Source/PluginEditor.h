/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================

//FFT Data Generator...

enum FFTOrder
{
    
    order2048 = 11,
    order4096 = 12,
    order8192 = 13
    
};

template<typename BlockType>
struct FFTDataGenerator
{
    
    //Produces the FFT data from an AudioBuffer
    
    void produceFFTDataForRendering (const juce::AudioBuffer<float>& audioData, const float negativeInfinity)
    {
        
        const auto fftSize = getFFTSize();
        
        fftData.assign(fftData.size(),0);
        auto* readIndex = audioData.getReadPointer(0);
        std::copy(readIndex, readIndex + fftSize, fftData.begin());
        
        //first apply a windowing fucntion to our data.
        window->multiplyWithWindowingTable(fftData.data(), fftSize);
        
        // Render the FFT data...
        forwardFFT->performFrequencyOnlyForwardTransform(fftData.data());
        
        int numBins = (int)fftSize/2;
        
        //normalize the FFT values...
        for (int i = 0; i < numBins; ++i)
        {
            auto v = fftData[i];

            
            if(!std::isinf(v) && !std::isnan(v))
            {
                
                v /= float(numBins);
                
            }
            else
            {
                v = 0.f;
            }
            
            fftData[i] = v;
            
        }
    
        //Convert them into decibels...
        for (int i = 0; i < numBins; ++i)
        {
            
            fftData[i] = juce::Decibels::gainToDecibels(fftData[i], negativeInfinity);
        }
        
        fftDataFifo.push(fftData);
        
    }
    
    void changeOrder(FFTOrder newOrder)
    {
        
        //When you create the order, recreate the window, forwardFFT, fifo, fftData
        //also reset the fifoIndex
        //things that need recreating should be created on the heap via std::make_unique<>
        
        order = newOrder;
        auto fftSize = getFFTSize();
        
        forwardFFT = std::make_unique<juce::dsp::FFT>(order);
        window = std::make_unique<juce::dsp::WindowingFunction<float>>(fftSize,
                                                                       juce::dsp::WindowingFunction<float>::blackmanHarris);
        
        fftData.clear();
        fftData.resize(fftSize * 2, 0);
        
        fftDataFifo.prepare(fftData.size());
        
    }
    
    //==============================================================================
    
    int getFFTSize() const { return 1 << order;}
    int getNumAvailableFFTDataBlocks() const { return fftDataFifo.getNumAvailableForReading();}
    
    //==============================================================================
    
    bool getFFTData(BlockType& fftData){return fftDataFifo.pull(fftData);}
    
private:
    FFTOrder order;
    BlockType fftData;
    std::unique_ptr<juce::dsp::FFT> forwardFFT;
    std::unique_ptr<juce::dsp::WindowingFunction<float>> window;
    
    Fifo<BlockType> fftDataFifo;
};

//==============================================================================
// Creating the FFT paths...

template <typename PathType>
struct AnalyzerPathGenerator
{
    
    /*
     converts "renderData[]" into a juce::Path
     */
    void generatePath(const std::vector<float>& renderData,
                      juce::Rectangle<float> fftBounds,
                      int fftSize,
                      float binWidth,
                      float negativeInfinity)
    {
        
        auto top = fftBounds.getY();
        auto bottom = fftBounds.getHeight();
        auto width = fftBounds.getWidth();
        
        int numBins = (int)fftSize/2;
        
        PathType p;
        
        p.preallocateSpace(3 * (int)fftBounds.getWidth());
        
        auto map = [bottom, top, negativeInfinity](float v)
            {
                
                return juce::jmap(v,
                                  negativeInfinity, 0.f,
                                  float(bottom+10), top);
            };
        
        auto y = map(renderData[0]);
        
       // jassert(!std::isnan(y) && !std::isinf(y));
        
        if(std::isnan(y) || std::isinf(y))
            y = bottom;
        
        p.startNewSubPath(0, y);
        
        const int pathResolution = 2; //you can draw line-to's every pathResolution pixels.
        
        for( int binNum = 1; binNum < numBins; binNum += pathResolution)
        {
            y = map(renderData[binNum]);
            
            // jassert( !std::isnan(y) && !std::isinf(y));
            
            if ( !std::isnan(y) && !std::isinf(y))
            {
                auto binFreq = binNum * binWidth;
                auto normalizedBinX = juce::mapFromLog10(binFreq, 20.f, 20000.f);
                int binX = std::floor(normalizedBinX * width);
                p.lineTo(binX,y);
                
            }
        }
        
        pathFifo.push(p);
        
    }
        
    
    int getNumPathsAvailable() const
    {
        
        return pathFifo.getNumAvailableForReading();
        
    }
    
    bool getPath(PathType& path)
    {
        return pathFifo.pull(path);
        
    }
    
private:
    
    Fifo<PathType> pathFifo;
    
};

//==============================================================================

//Look and Feel de RotarySliderWithLabels

struct LookAndFeel: juce::LookAndFeel_V4
{
    
    void drawRotarySlider (juce::Graphics&,
                                   int x, int y, int width, int height,
                                   float sliderPosProportional,
                                   float rotaryStartAngle,
                                   float rotaryEndAngle,
                           juce::Slider&) override;
   
    
    
    void drawToggleButton (juce::Graphics& g,
                           juce::ToggleButton& toggleButton,
                           bool shouldDrawButtonAsHighlighted,
                           bool shouldDrawButtonAsDown
                           ) override;
    
    
    
    
};

struct RotarySliderWithLabels: juce::Slider
{
    //constructor y destructor
    RotarySliderWithLabels(juce::RangedAudioParameter &rap, const juce::String unitSuffix):
    juce::Slider (juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                  juce::Slider::TextEntryBoxPosition::NoTextBox),
    param(&rap),
    suffix(unitSuffix)
                                            
        {
            setLookAndFeel(&lnf);
        }
    
    ~RotarySliderWithLabels()
    {
        setLookAndFeel(nullptr);
    }
    
    // Etiquetas de valores minimos y máximos...
    
    struct LabelPos
    {
        float pos; //normalized possition
        juce::String label; //label name ex:  20 Hz - 20kHz.
    };
    
    juce::Array<LabelPos> labels;
    
    //Metodo de paint
    void paint(juce::Graphics &g)override;
    
    //TextBox
    juce::Rectangle<int> getSliderBounds() const;
    int getTextHeight() const {return 14;}
    juce::String getDisplayString() const;
    
    
private:
    //Instancias para el LookAndFeel
    juce::RangedAudioParameter* param;
    juce::String suffix;
    LookAndFeel lnf;
    
};

//==============================================================================

struct PathProducer
{
    
    PathProducer(SingleChannelSampleFifo<EelEQAudioProcessor::BlockType>& scsf) :
    leftChannelFifo(&scsf)
    {
        leftChannelFFTDataGenerator.changeOrder(FFTOrder::order4096);
        monoBuffer.setSize(1, leftChannelFFTDataGenerator.getFFTSize());
                
    }
    
    void process(juce::Rectangle<float> fftBounds, double sampleRate);
    juce::Path getPath(){ return leftChannelFFTPath; }
    
private:
    
    SingleChannelSampleFifo<EelEQAudioProcessor::BlockType>* leftChannelFifo;
    juce::AudioBuffer<float> monoBuffer;
    FFTDataGenerator<std::vector<float>> leftChannelFFTDataGenerator;
    AnalyzerPathGenerator<juce::Path> pathProducer;
    juce::Path leftChannelFFTPath;

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
    
    // Listener and Timer Callbacks
    void parameterValueChanged (int parameterIndex, float newValue) override;
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override{ };
    void timerCallback() override;
    
    //Herencia del editor.
    void paint (juce::Graphics& g) override;
    void resized()override;
    
private:
    
    EelEQAudioProcessor& audioProcessor;
    
    //Atomic Timer
    juce::Atomic<bool> parametersChanged {false};

    
    //BG IMAGE
    juce::Image background;
    juce::Rectangle<int> getRenderArea();
    juce::Rectangle<int> getAnalysisArea();
    
    juce::Path responseCurve;
    
    void updateResponseCurve();
    void drawBackgroundGrid(juce::Graphics& g);
    void drawTextLabels(juce::Graphics& g);
    
    std::vector<float> getFrequencies();
    std::vector<float> getGains();
    std::vector<float> getXs(const std::vector<float>& freqs, float left, float width);
    
    //MonoChain
    MonoChain monoChain;
    void UpdateChain();
    
    //FFT
    PathProducer leftPathProducer, rightPathProducer;
    
};
//==============================================================================
// Bypass buttons for Filters N the FFT Analyzer...

struct PowerButton : juce::ToggleButton {};
struct AnalyzerButton : juce::ToggleButton
{
    void resized() override
    {
        auto bounds = getLocalBounds();
        auto insetRect = bounds.reduced(4);
        
        randomPath.clear();
        
        juce::Random r;
        
        //Crear los puntos
        randomPath.startNewSubPath(insetRect.getX(),
                                   insetRect.getY() + insetRect.getHeight() * r.nextFloat());
        
        //Unir puntos
        for (auto x = insetRect.getX() + 1; x < insetRect.getRight(); x += 2 )
        {
            randomPath.lineTo(x,
                              insetRect.getY() + insetRect.getHeight() * r.nextFloat());
        }
        
    }
      
    juce::Path randomPath;
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
    RotarySliderWithLabels peakFreqSlider,
    peakGainSlider,
    peakQualitySlider,
    lowcutFreqSlider,
    highcutFreqSlider,
    lowcutSlopeSlider,
    highcutSlopeSlider;
    
    //ResponseCurveComponent
    ResponseCurveComponent responseCurveComponent;
    
    // Namespace para obtener los parametros
    
    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;
    
    Attachment peakFreqSliderAttachment,
    peakGainSliderAttachment,
    peakQualitySliderAttachment,
    lowcutFreqSliderAttachment,
    highcutFreqSliderAttachment,
    lowcutSlopeSliderAttachment,
    highcutSlopeSliderAttachment;
    
    // Funcion Auxiliar para modificar los sliders
    
    std::vector<juce::Component*> getComps();
    
    
    //Botones de Bypass...
    
    PowerButton lowCutBypassButton, highCutBypassButton, peakBypassButton;
    AnalyzerButton analyzerEnabledButton;
    
    using ButtonAttachment = APVTS::ButtonAttachment;
    
    ButtonAttachment lowCutBypassButtonAttachment,
    highCutBypassButtonAttachment,
    peakButtonBypassAttachment,
    analyzerEnabledButtonAttachment;
    
    LookAndFeel lnf;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EelEQAudioProcessorEditor);
};
