/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
//LookAndFeel

void LookAndFeel::drawRotarySlider(juce::Graphics& g,
                                   int x, int y, int width, int height,
                                   float sliderPosProportional,
                                   float rotaryStartAngle,
                                   float rotaryEndAngle,
                                   juce::Slider& slider){
    
    using namespace juce;
    
    auto bounds = Rectangle<float>(x, y, width, height);
    
    // This colors the button
    g.setColour(Colour(97u,18u,167u));
    g.fillEllipse(bounds);
    g.setColour(Colours::black);
    g.drawEllipse(bounds,  1.f);
    
    // Adding the parameter values to the GUI
    
    if(auto* rswl =dynamic_cast<RotarySliderWithLabels*>(&slider)){
        
        auto center = bounds.getCentre();
        Path p;
        
        Rectangle<float> r;
        r.setLeft(center.getX()-5);
        r.setRight(center.getX()+5);
        r.setTop(bounds.getY());
        r.setBottom(center.getY() - rswl->getTextHeight()*1.5); // addapt the bottom in relation to the text.
        
        p.addRoundedRectangle(r, 2.f);
        
        //This gets the center to the pointer rectangle.
        
        jassert(rotaryStartAngle<rotaryEndAngle); //catching errors.
        
        auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle); //mapear los valores de posisción entre el angulo de inicio y el angulo final.
        
        p.applyTransform(AffineTransform().rotated(sliderAngRad,center.getX(), center.getY()));
        
        g.fillPath(p);
        
        // TEXT and TEXTBOX
        
        g.setFont(rswl->getTextHeight()*0.90);
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);
        
        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(bounds.getCentre());
        
        g.setColour(Colours::black);
        g.fillRect(r);
        
        g.setColour(Colours::white);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1); // Poner el Texto en el rectangulo
    }
    
    
}



//==============================================================================
//RotarySliderWithLabel

void RotarySliderWithLabels::paint(juce::Graphics &g){
    using namespace juce;
    
    // we set de angles of the 0 and the 1 values
    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;
    
    auto range = getRange(); //need implementation for normalized values
    auto sliderBounds = getSliderBounds();
    
    // Helpers to get the proportion of the ButtonSliders (uncomment for show 'em)....
//    g.setColour(Colours::red);
//    g.drawRect(getLocalBounds());
//    g.setColour(Colours::yellow);
//    g.drawRect(sliderBounds);
    
    
    getLookAndFeel().drawRotarySlider(g,
                                      sliderBounds.getX(),
                                      sliderBounds.getY(),
                                      sliderBounds.getWidth(),
                                      sliderBounds.getHeight(),
                                      jmap(getValue(),range.getStart(),range.getEnd(),0.0,1.0), // here is where we Normalize the slider value
                                      startAng,
                                      endAng,
                                      *this);
    
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds()const
{
    
    auto bounds = getLocalBounds();
    
    auto size = juce::jmin(bounds.getWidth(),bounds.getHeight());
    
    size -= getTextHeight()*2;
    
    juce::Rectangle<int> r;
    
    r.setSize(size,size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);
    
    return r;
    
}



// Implement the string
juce::String RotarySliderWithLabels::getDisplayString() const {
    
// Condition to display choices if a the parameter is a choice based parameter....
    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
        return choiceParam ->getCurrentChoiceName();
        
// Here we display the units for the parameter values...
    
    
    juce::String str;
    bool addK = false; // for adding the k prefix to the Hz
    
    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param))
    {
        float val = getValue();
        
        if ( val > 999.f)
        {
            val /= 1000.f;
            addK = true;
        }
        
        // Values to strings
        
        str = juce::String(val, (addK ? 2:0)); // Number of decimal places, if the value is below 1000, there are no decimal places (0).
        
    }
    
    else
    {
        jassertfalse; //This shouldnt happen.
    }
    
    
    if (suffix.isNotEmpty()){
        
        str<<" ";
        if(addK){
            str<<"k";
        }
        str<<suffix;
        
    }
    
    return str;
    
}





//==============================================================================
//ResponseCurveComponent

ResponseCurveComponent::ResponseCurveComponent(EelEQAudioProcessor& p): audioProcessor(p){
    
    // Add listeer
    
    const auto& params = audioProcessor.getParameters();
    for(auto param: params){
        param->addListener(this);
    }
    
    // start the timer (very importante)
    startTimerHz(60);
    
}

ResponseCurveComponent::~ResponseCurveComponent()
{
    
    //Remove listener on exit...
    const auto& params = audioProcessor.getParameters();
    for(auto param: params){
        param->removeListener(this);
    }
    
}

void ResponseCurveComponent::paint (juce::Graphics& g)
{
    
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::black);
    
    //Drawing the response curve...
    
    //We need to get the bounds and the width of the response area.
    
    auto responseArea = getLocalBounds();
    
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

void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue){
    
    // Set the atomic flag
    parametersChanged.set(true);
    
}

void ResponseCurveComponent::timerCallback(){
    
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
        
        UpdateCutFilter(monoChain.get<ChainPositions::LowCut>(), lowcutCoefficients, chainSettings.lowCutSlope);
        UpdateCutFilter(monoChain.get<ChainPositions::HighCut>(), highcutCoefficients, chainSettings.highCutSlope);
        
     
        // Redibujar la curva
        repaint();
        
        
    }

}


//==============================================================================
EelEQAudioProcessorEditor::EelEQAudioProcessorEditor(EelEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    peakFreqSlider(*audioProcessor.apvts.getParameter("Peak Freq"), "Hz"),
    peakGainSlider(*audioProcessor.apvts.getParameter("Peak Gain"), "dB"),
    peakQualitySlider(*audioProcessor.apvts.getParameter("Quality"), "Q"),
    lowcutFreqSlider(*audioProcessor.apvts.getParameter("LowCut Freq"), "Hz"),
    highcutFreqSlider(*audioProcessor.apvts.getParameter("HighCut Freq"), "Hz"),
    lowcutSlopeSlider(*audioProcessor.apvts.getParameter("LowCut Slope"), "dB/Oct"),
    highcutSlopeSlider(*audioProcessor.apvts.getParameter("HighCut Slope"), "dB/Oct"),
    responseCurveComponent(audioProcessor),
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
    
    setSize (600, 400);
}

EelEQAudioProcessorEditor::~EelEQAudioProcessorEditor()
{

    
}

//==============================================================================
void EelEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::black);

    
}

void EelEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight()*0.33);
    responseCurveComponent.setBounds(responseArea);
    
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



std::vector<juce::Component*> EelEQAudioProcessorEditor::getComp(){
    
    return
    {
        &peakFreqSlider,
        &peakGainSlider,
        &peakQualitySlider,
        &lowcutFreqSlider,
        &highcutFreqSlider,
        &lowcutSlopeSlider,
        &highcutSlopeSlider,
        &responseCurveComponent
        
        
    };
    
}


