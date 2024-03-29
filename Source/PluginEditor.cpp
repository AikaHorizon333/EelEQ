/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//================================================================================
//==================================LookAndFeel===================================
//================================================================================

void LookAndFeel::drawRotarySlider(juce::Graphics& g,
                                   int x, int y, int width, int height,
                                   float sliderPosProportional,
                                   float rotaryStartAngle,
                                   float rotaryEndAngle,
                                   juce::Slider& slider)
{
    
    using namespace juce;
    
    auto bounds = Rectangle<float>(x, y, width, height);
    
    auto enabled = slider.isEnabled();
    
    // This colors the button
    g.setColour(enabled ? Colour(97u,18u,167u) : Colours::darkgrey);
    g.fillEllipse(bounds);// Relleno del Boton
    
    g.setColour(enabled ? Colours::lavender : Colours::grey);
    g.drawEllipse(bounds, 1.5f); // borde del boton
    
    // Adding the parameter values to the GUI
    
    if(auto* rswl =dynamic_cast<RotarySliderWithLabels*>(&slider)){
        
        auto center = bounds.getCentre();
        Path p;
        
        Rectangle<float> r;
        r.setLeft(center.getX() - 5);
        r.setRight(center.getX() + 5 );
        r.setTop(bounds.getY());
        r.setBottom(center.getY() - rswl->getTextHeight() * 2.5); // addapt the bottom in relation to the text.
        
        p.addRoundedRectangle(r, 1.f);
        
        //This gets the center to the pointer rectangle:
        
        jassert(rotaryStartAngle < rotaryEndAngle);
        
        //mapear los valores de posisción entre el angulo de inicio y el angulo final.
        auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);
        
        p.applyTransform(AffineTransform().rotated(sliderAngRad,center.getX(), center.getY()));
        
        g.fillPath(p);
        
        // TEXT and TEXTBOX
        
        g.setFont(rswl->getTextHeight()*0.90);
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);
        
        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(bounds.getCentre());
        
        g.setColour(enabled? Colours::black : Colours::darkgrey);
        g.fillRect(r);
        
        g.setColour(enabled ? Colours::white : Colours::lightgrey);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1); // Esto pone el Texto en el rectangulo
    }
    
}

//==============================================================================
//Bypass Button Look And Feel...

void LookAndFeel::drawToggleButton(juce::Graphics &g,
                                   juce::ToggleButton &toggleButton,
                                   bool shouldDrawButtonAsHighlighted,
                                   bool shouldDrawButtonAsDown)
{
    
    using namespace juce;
    
    if(auto* pb = dynamic_cast<PowerButton*>(&toggleButton))
    {
        Path powerButton;
        
        auto bounds = toggleButton.getLocalBounds();
        
    //GUI helper: Uncommet to see them...
//        g.setColour(Colours::red);
//        g.drawRect(bounds);
        
        auto size = jmin(bounds.getWidth(), bounds.getHeight() - 6);
        auto r = bounds.withSizeKeepingCentre(size, size).toFloat();
        
        float ang = 30.f;
        size -= 6;
        
        //Dibujar el boton de encendido...
        //arco
        powerButton.addCentredArc(r.getCentreX(),
                                  r.getCentreY(),
                                  size * 0.5,
                                  size * 0.5,
                                  0.f,
                                  degreesToRadians(ang),
                                  degreesToRadians(360.f - ang),
                                  true);
        
        //linea Vertical
        powerButton.startNewSubPath(r.getCentreX(), r.getY());
        powerButton.lineTo((r.getCentre()));
        PathStrokeType pst(2.f, PathStrokeType::JointStyle::curved);
        
        
        // Está encendido o apagado
        auto color = toggleButton.getToggleState() ? Colours::dimgrey : Colours::yellowgreen;
        
        g.setColour(color);
        g.strokePath(powerButton, pst);
        g.drawEllipse(r, 2.f);
    }
    
    else if (auto* analyzerButton = dynamic_cast<AnalyzerButton*>(&toggleButton))
    {
        
        //Queremos dibujar
        auto color = ! toggleButton.getToggleState() ? Colours::dimgrey : Colours::yellowgreen;
        
        g.setColour(color);
        
        auto bounds = toggleButton.getLocalBounds();
        g.drawRect(bounds);
        
        g.strokePath(analyzerButton->randomPath, PathStrokeType(1.f));
        
    }
}

//================================================================================
//==========================RotarySliderWithLabels================================
//================================================================================


void RotarySliderWithLabels::paint(juce::Graphics &g)
{
    using namespace juce;
    
    // we set de angles of the 0 and the 1 values
    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;
    
    auto range = getRange(); //need implementation for normalized values
    auto sliderBounds = getSliderBounds();
    
//Helpers to get the proportion of the ButtonSliders (uncomment for show 'em)....
    
//    g.setColour(Colours::red);
//    g.drawRect(getLocalBounds());
//    g.setColour(Colours::yellow);
//    g.drawRect(sliderBounds);
    
    
    getLookAndFeel().drawRotarySlider(g,
                                      sliderBounds.getX(),
                                      sliderBounds.getY(),
                                      sliderBounds.getWidth(),
                                      sliderBounds.getHeight(),
                                      jmap(getValue(),range.getStart(),range.getEnd(),0.0,1.0),
                                      startAng,
                                      endAng,
                                      *this);
    
// Dibujar los limites minimos y máximos de los sliders...
    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth()*0.5;
    
    g.setColour(Colours::yellowgreen);
    g.setFont(getTextHeight());
    
    auto numChoices = labels.size();
    
    for(int i =0; i < numChoices ; ++i)
    {
        
        auto pos =labels[i].pos;
        
        jassert(0.f <= pos);
        jassert(pos <= 1.f);
        
        auto ang = jmap(pos, 0.f, 1.f, startAng, endAng);  // mapeo de los min y max de los sliders...
        
        auto c = center.getPointOnCircumference(radius + getTextHeight() * 0.5 + 1, ang); // fijar un punto para dibujar el texto.
        
        Rectangle<float> r;
        auto str = labels[i].label;
        r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
        r.setCentre(c);
        r.setY(r.getY() + getTextHeight());
        
        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
        
        // Recuerda que hay que inicializar los labels desde el constructor.
        
    }
    
}

//Obtener los bordes de los sliders para colocarlos.
juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds()const
{
    
    auto bounds = getLocalBounds();
    
    auto size = juce::jmin(bounds.getWidth(),bounds.getHeight());
    
    size -= getTextHeight() * 2;
    
    juce::Rectangle<int> r;
    r.setSize(size,size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);
    
    return r;
    
}



// Implement the string
juce::String RotarySliderWithLabels::getDisplayString() const
{
    
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
        
        // Number of decimal places, if the value is below 1000, there are no decimal places (0).
        str = juce::String(val, (addK ? 2:0));
    }

    else
    {
        jassertfalse; //This shouldnt happen.
    }
    
    if (suffix.isNotEmpty())
    {
        str<<" ";
        if(addK){
            str<<"k";
        }
        str<<suffix;
    }
    
    return str;
    
}

//================================================================================
//==========================ResponseCurveComponent================================
//================================================================================

ResponseCurveComponent::ResponseCurveComponent(EelEQAudioProcessor& p):
audioProcessor(p),
leftPathProducer(audioProcessor.leftChannelFifo),
rightPathProducer(audioProcessor.rightChannelFifo)
{
    // Add listener
    const auto& params = audioProcessor.getParameters();
    
    for(auto param: params)
    {
        param->addListener(this);
    }
    
    //Paint the current parameters.
    UpdateChain();
    
    // start the timer (very importante)
    startTimerHz(60);
    
}


ResponseCurveComponent::~ResponseCurveComponent()
{
    //Remove listener on exit...
    const auto& params = audioProcessor.getParameters();
    for(auto param: params)
    {
        param->removeListener(this);
    }
}

//========================================================================
void ResponseCurveComponent::updateResponseCurve()
{
    using namespace juce;
    
    //Drawing the response curve...
    
    //We need to get the bounds and the width of the response area.
    
    auto responseArea = getAnalysisArea();
    
    auto w = responseArea.getWidth();
    
    auto& lowcut = monoChain.get<ChainPositions::LowCut>();
    auto& peak = monoChain.get<ChainPositions::Peak>();
    auto& highcut = monoChain.get<ChainPositions::HighCut>();
    
    
    auto sampleRate = audioProcessor.getSampleRate();
    
    std::vector<double> mags;
    
    mags.resize(w);
    
    for(int i = 0; i<w; ++i)
    {
        double mag = 1.f;
       
        //mapping the frequency to human hearing range.
        auto freq = mapToLog10(double(i)/double(w), 20.0, 20000.0);
        
        //peak
        if( !monoChain.isBypassed<ChainPositions::Peak>() )
            mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        
        
        //lowcuts
        if( !monoChain.isBypassed<ChainPositions::LowCut>() )
        {
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
        }
        
        //highcut
        if( !monoChain.isBypassed<ChainPositions::HighCut>() )
        {
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
        }
        
        mags[i] = Decibels::gainToDecibels(mag);
        
    }
    
    responseCurve.clear();
    
    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    auto map = [outputMin, outputMax](double input)
    {
        return jmap(input, -24.0, 24.0, outputMin, outputMax);
    };
    
    //this starts the mapping
    responseCurve.startNewSubPath(responseArea.getX(), map(mags.front())); // this will run the first value of the jmap.
    
    //this checks the values and maps them into the response curve.
    for( size_t i = 1 ; i < mags.size(); ++i)
    {
        responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
    }
    
    
}

//========================================================================

void ResponseCurveComponent::paint (juce::Graphics& g)
{
    
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::black);
    
    drawBackgroundGrid(g);
   
    //Drawing the FFT....
    
    
    auto responseArea = getAnalysisArea();
    
    
    
    //Adding the Bypass condition
    
    if(shouldShowFFTAnalysis)
    {
        //LEFT
        auto leftChannelFFTPath = leftPathProducer.getPath();
        leftChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));
        g.setColour(Colours::blue);
        g.strokePath(leftChannelFFTPath, PathStrokeType(1.f));
        
        //RIGHT
        auto rightChannelFFTPath = rightPathProducer.getPath();
        rightChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));
        g.setColour(Colours::red);
        g.strokePath(rightChannelFFTPath, PathStrokeType(1.f));
    }
    
    //Drawing the Response Curve Path
    g.setColour(Colours::white);
    g.strokePath(responseCurve, PathStrokeType(2.f));
    
    //Border
    Path border;
    
    border.setUsingNonZeroWinding(false);
    
    border.addRoundedRectangle(getRenderArea(),4);
    border.addRectangle(getLocalBounds());
    
    g.setColour(Colours::black);
    g.fillPath(border);
    
    
    //Text Labels aux funtion...
    drawTextLabels(g);
    
    //Orange Rectangle...
    g.setColour(Colours::orange);
    g.drawRoundedRectangle(getRenderArea().toFloat(), 4.f, 1.f);
    
}

std::vector<float> ResponseCurveComponent::getFrequencies()
{
    
    return std::vector<float>
    {
        20,50,100,
        200,500,1000,
        2000,5000,10000,
        20000
        
    };
}

std::vector<float> ResponseCurveComponent::getGains()
{
    return std::vector<float>
    {
        24.f, 12.f, 0.f, -12.f, -24.f
        
    };
}

std::vector<float> ResponseCurveComponent::getXs(const std::vector<float> &freqs, float left, float width)
{
    std::vector<float> xs;
    
    for (auto f : freqs)
    {
        auto normX = juce::mapFromLog10(f, 20.f, 20000.f);
        xs.push_back(left + width * normX);
        
    }
    return xs;
    
}

void ResponseCurveComponent::drawBackgroundGrid(juce::Graphics &g)
{
    using namespace juce;
    
    auto freqs = getFrequencies();
    
    auto renderArea = getAnalysisArea();
    auto left = renderArea.getX();
    auto right = renderArea.getRight();
    auto top = renderArea.getY();
    auto bottom = renderArea.getBottom();
    auto width = renderArea.getWidth();
    
    auto xs = getXs(freqs, left, width);  // Normalizamos los valores de X
    
    g.setColour(Colours::dimgrey);
    
    for (auto x : xs)
    {
        g.drawVerticalLine(x, top, bottom); // Dibujamos las Frecuencias en X.
        
    }
    
    auto gain = getGains();
    
    for( auto gdB : gain)
    {
        auto y = jmap(gdB, -24.f, 24.f, float(bottom), float(top));
        
        g.setColour(gdB == 0.f ? Colours::darkgreen : Colours::dimgrey);
        g.drawHorizontalLine(y, left, right); //Dibujamos las lineas de ganancia en Y.
        
    }
    
}

void ResponseCurveComponent::drawTextLabels(juce::Graphics &g)
{
    using namespace juce;
//  Dibujamos los labels...
//  Seleccionamos un color y un tamaño de letra...
    
    g.setColour(Colours::lightgrey);
    const int fontHeight = 10;
    g.setFont(fontHeight);
    
// Sacamos las constantes...
    
    auto renderArea = getAnalysisArea();
    auto left = renderArea.getX();
    auto top = renderArea.getY();
    auto bottom = renderArea.getBottom();
    auto width = renderArea.getWidth();
    
    auto freqs = getFrequencies();
    auto xs = getXs(freqs, left, width);
    
    for (int i = 0; i < freqs.size(); ++i)
        {
            auto f = freqs[i];
            auto x = xs[i];
            
            bool addK = false;
            String str;
            if( f > 999.f )
            {
                addK = true;
                f /= 1000.f;
            }
            
            
            str << f;
            
            if( addK )
            {
                str << "k";
            }
            
            str << "Hz";
            
            auto textWidth = g.getCurrentFont().getStringWidth(str);
            
            Rectangle<int> r;
            r.setSize(textWidth, fontHeight);
            r.setCentre(x, 0);
            r.setY(1);
            
            g.drawFittedText(str,r,juce::Justification::centred, 1);
        
        }
        
    auto gain = getGains();
    
    for (auto gdB : gain)
    {
            //Coordenadas de mapeo para las labels de ganacia..
            auto y = jmap(gdB, -24.f, 24.f, float(bottom),float(top));
            
            String str;
            if( gdB> 0)
                str << "+";
            str << gdB;
            
            auto textWidth = g.getCurrentFont().getStringWidth(str);
            
            //textbox para la ganancia del EQ
            Rectangle<int> r;
            r.setSize(textWidth, fontHeight);
            r.setX(getWidth() - textWidth);
            r.setCentre(r.getCentreX(), y);
            
            g.setColour(gdB == 0 ? Colours::darkgreen : Colours::lightgrey);
            
            g.drawFittedText(str,r, juce::Justification::centred, 1);
            
            //textbox para el analizador de frecuencia
            str.clear();
            str << (gdB -24.f);
            
            //Actualizamos el tamaño del str a mostrar
            textWidth = g.getCurrentFont().getStringWidth(str);
            
            r.setSize(textWidth, fontHeight);
            r.setX(1);
            
            //Actualizamos los colores
            g.setColour(Colours::lightgrey);
            g.drawFittedText(str, r, juce::Justification::centred, 1);
            
        }
    
}

void ResponseCurveComponent::resized()
{
    using namespace juce;
    
    responseCurve.preallocateSpace(getWidth() * 3);
    
    updateResponseCurve();

}

void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue)
{
    
    // Set the atomic flag
    parametersChanged.set(true);
    
}

void PathProducer::process(juce::Rectangle<float> fftBounds, double sampleRate)
{
    juce::AudioBuffer<float> tempIncomingBuffer;
    
    while(leftChannelFifo -> getNumCompleteBuffersAvailable() > 0)
    {
        
        if (leftChannelFifo->getAudioBuffer(tempIncomingBuffer))
        {
            //Vamos a recorrer el buffer el numero de samples que estan en el buffer temporal
            auto size = tempIncomingBuffer.getNumSamples();
            
            // copiamos el buffer y lo recorremos #size samples a la izquierda
            juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0,0),
                                              monoBuffer.getReadPointer(0,size),
                                              monoBuffer.getNumSamples() - size );
            
            juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, monoBuffer.getNumSamples() - size),
                                              tempIncomingBuffer.getReadPointer(0, 0),
                                              size);
      
            leftChannelFFTDataGenerator.produceFFTDataForRendering(monoBuffer, -48.f);
            
            // -48 represents the -infinity. also is the bottom of the display...

        }
    }
    
    
    auto fftSize = leftChannelFFTDataGenerator.getFFTSize();
    
    const auto binWidth = sampleRate / double(fftSize);
    
    while (leftChannelFFTDataGenerator.getNumAvailableFFTDataBlocks() > 0)
    {
        std::vector<float> fftData;
        if(leftChannelFFTDataGenerator.getFFTData(fftData))
        {
            pathProducer.generatePath(fftData, fftBounds, fftSize, binWidth, -48.f);
        }
    }
    
    //Actualizar los paths y utilizar los más recientes...
    
    while (pathProducer.getNumPathsAvailable() > 0)
    {
        pathProducer.getPath(leftChannelFFTPath);
    }
    
}


void ResponseCurveComponent::timerCallback()
{
    
    // Bypasseamos el proceso de la FFT aquí....
    if(shouldShowFFTAnalysis)
    {
        auto fftBounds = getAnalysisArea().toFloat();
        auto sampleRate = audioProcessor.getSampleRate();
        
        leftPathProducer.process(fftBounds, sampleRate);
        rightPathProducer.process(fftBounds, sampleRate);
    }
    
    // Solo va a actualizar si se realizó algun cambio en el parametro
    if(parametersChanged.compareAndSetBool(false, true))
    {
        // Actualizar el monoChain
        
        DBG("Params Change");
        UpdateChain();
        updateResponseCurve();
        
    }
    
    repaint();
}

void ResponseCurveComponent::UpdateChain()
{
    
    auto chainSettings = getChainSettings(audioProcessor.apvts);
    
    monoChain.setBypassed<ChainPositions::LowCut>(chainSettings.lowCutBypassed);
    monoChain.setBypassed<ChainPositions::HighCut>(chainSettings.highCutBypassed);
    monoChain.setBypassed<ChainPositions::Peak>(chainSettings.peakBypassed);
    
    auto peakCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate());
    UpdateCoefficients(monoChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
    
    auto lowcutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
    auto highcutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());
    
    UpdateCutFilter(monoChain.get<ChainPositions::LowCut>(), lowcutCoefficients, chainSettings.lowCutSlope);
    UpdateCutFilter(monoChain.get<ChainPositions::HighCut>(), highcutCoefficients, chainSettings.highCutSlope);
    
    
}

juce::Rectangle <int> ResponseCurveComponent::getRenderArea()
{
    auto bounds = getLocalBounds();
    
    bounds.removeFromTop(12);
    bounds.removeFromBottom(2);
    bounds.removeFromLeft(20);
    bounds.removeFromRight(20);

    return bounds;
}

juce::Rectangle<int> ResponseCurveComponent::getAnalysisArea()
{
    auto bounds = getRenderArea();
    bounds.removeFromTop(4);
    bounds.removeFromBottom(4);
    
    return bounds;
    
}

//================================================================================
//==========================EelEQAudioProcessorEditor=============================
//================================================================================

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
    highcutSlopeSliderAttachment(audioProcessor.apvts, "HighCut Slope", highcutSlopeSlider),

    lowCutBypassButtonAttachment(audioProcessor.apvts, "LowCut Bypassed", lowCutBypassButton),
    highCutBypassButtonAttachment(audioProcessor.apvts, "HighCut Bypassed", highCutBypassButton),
    peakButtonBypassAttachment(audioProcessor.apvts, "Peak Bypassed", peakBypassButton),
    analyzerEnabledButtonAttachment(audioProcessor.apvts, "Analyzer Enabled", analyzerEnabledButton)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
// Inicialización de los Labels
    
    peakFreqSlider.labels.add({0.f,"20 Hz"});
    peakFreqSlider.labels.add({1.f,"20 kHz"});
    
    peakGainSlider.labels.add({0.f,"-24.0 dB"});
    peakGainSlider.labels.add({1.f,"24.0 dB"});
    
    peakQualitySlider.labels.add({0.f,"0.1 Q"});
    peakQualitySlider.labels.add({1.f,"15.0 Q"});
    
    
    lowcutFreqSlider.labels.add({0.f,"20 Hz"});
    lowcutFreqSlider.labels.add({1.f,"20 kHz"});
    
    lowcutSlopeSlider.labels.add({0.f,"12"});
    lowcutSlopeSlider.labels.add({1.f,"48"});
    
    highcutFreqSlider.labels.add({0.f,"20 Hz"});
    highcutFreqSlider.labels.add({1.f,"20 kHz"});
    
    highcutSlopeSlider.labels.add({0.f,"12"});
    highcutSlopeSlider.labels.add({1.f,"48"});
    
    
    for(auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }
    
    //Dibujar los botones de Bypass
    peakBypassButton.setLookAndFeel(&lnf);
    lowCutBypassButton.setLookAndFeel(&lnf);
    highCutBypassButton.setLookAndFeel(&lnf);
    analyzerEnabledButton.setLookAndFeel(&lnf);
    
    
    
    // Desactivar Componentes...
    
    auto safePtr = juce::Component::SafePointer<EelEQAudioProcessorEditor>(this);
    
    
    // Conectamos el safePtr al boton de bypass para poder meternos a sus componentes...
    
    peakBypassButton.onClick = [safePtr]()
    {
        if( auto* comp = safePtr.getComponent()) //Revisamos la existencia del componente...
        {
            auto bypassed = comp->peakBypassButton.getToggleState();
            
            comp->peakFreqSlider.setEnabled(!bypassed);
            comp->peakGainSlider.setEnabled(!bypassed);
            comp->peakQualitySlider.setEnabled(!bypassed);
            
            
        }
    };
    
    // Repetimos con los componentes que queramos desactivar...
    lowCutBypassButton.onClick = [safePtr]()
    {
        if (auto* comp = safePtr.getComponent())
        {
            auto bypassed = comp->lowCutBypassButton.getToggleState();
            
            comp->lowcutFreqSlider.setEnabled(!bypassed);
            comp->lowcutSlopeSlider.setEnabled(!bypassed);
            
        }
        
    };
    
    highCutBypassButton.onClick = [safePtr]()
    {
        if (auto* comp = safePtr.getComponent())
        {
            auto bypassed = comp->highCutBypassButton.getToggleState();
            
            comp->highcutFreqSlider.setEnabled(!bypassed);
            comp->highcutSlopeSlider.setEnabled(!bypassed);
        }
    };
    
    analyzerEnabledButton.onClick = [safePtr]()
    {
        if (auto* comp = safePtr.getComponent())
        {
            auto enabled = comp->analyzerEnabledButton.getToggleState();
            
            comp->responseCurveComponent.toggleAnalysisEnablement(enabled);
            
            
        }
    };
    
    
    
    // Modificamos el aspecto del plugin
    
    setSize (488, 600);
}

EelEQAudioProcessorEditor::~EelEQAudioProcessorEditor()
{
    
    peakBypassButton.setLookAndFeel(nullptr);
    lowCutBypassButton.setLookAndFeel(nullptr);
    highCutBypassButton.setLookAndFeel(nullptr);
    analyzerEnabledButton.setLookAndFeel(nullptr);
    
    
}

//==============================================================================
void EelEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    
    using namespace juce;

    //Vamos a pintar el plugin:
    
    g.fillAll (Colours::black);

    Path curve;

    auto bounds = getLocalBounds(); //Limites del Plugin
    auto center = bounds.getCentre(); // Coordenadas del centro

    // Creamos un titulo para el Header del plugin...
    String title {"EEL 01 EQ"};

    // Dibujamos el titulo...
    g.setFont (Font ("Arial Black", 25, Font::plain));
    
    
    // dibujamos el Header...
    auto titleWidth = g.getCurrentFont().getStringWidth(title);
        
    curve.startNewSubPath(center.x, 32);
    curve.lineTo(center.x - titleWidth * 0.45, //0.45
                 32);
    
    auto cornerSize = 40;
    auto curvePos = curve.getCurrentPosition();
    
    curve.quadraticTo(curvePos.getX() - cornerSize, curvePos.getY(),
                          curvePos.getX() - cornerSize, curvePos.getY() - 16);
    
    curvePos = curve.getCurrentPosition();
    
    
    curve.quadraticTo(curvePos.getX(), 2,
                        curvePos.getX() - cornerSize, 2);
        
    curve.lineTo({0.f, //0.f
                2.f}); //2.f
    
    curve.lineTo(0.f,   //0.f
                 0.f);  //0.f
    
    curve.lineTo(center.x, 0.f); //0.f
    curve.closeSubPath();
        
    g.setColour(Colour(97u, 18u, 167u));
    g.fillPath(curve);
        
    curve.applyTransform(AffineTransform().scaled(-1, 1));
    curve.applyTransform(AffineTransform().translated(getWidth(), 0));
    g.fillPath(curve);

    //Ponemos El titulo
    g.setColour(Colours::orange);
    g.drawFittedText(title, bounds, juce::Justification::centredTop, 1);
    
    
    
    
    
    
    
    //Nombre de los parametros...
    g.setColour(Colours::lightgrey);
    g.setFont(16);
    g.drawFittedText("LowCut", lowcutSlopeSlider.getBounds(), juce::Justification::centredBottom, 1);
    g.drawFittedText("Peak", peakQualitySlider.getBounds(), juce::Justification::centredBottom, 1);
    g.drawFittedText("HighCut", highcutSlopeSlider.getBounds(), juce::Justification::centredBottom, 1);
        
    //Fecha de Compilado..
    
    auto buildDate = Time::getCompilationDate().toString(true, false);
    auto buildTime = Time::getCompilationDate().toString(false, true);
    
    g.setFont(Font("Arial", 7, Font::italic));
    g.drawFittedText("Build: " + buildDate + "\n" + buildTime,
                     highcutSlopeSlider.getBounds().withY(6),
                     Justification::topRight,
                     2);

    
    
    
    
    
    
    
    

}

void EelEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    auto bounds = getLocalBounds();
    
    //Creación del boton
    auto analyzerEnabledArea = bounds.removeFromTop(25); //Area
    analyzerEnabledArea.setWidth(100); //ancho
    analyzerEnabledArea.setX(5); //donde empieza su eje x
    analyzerEnabledArea.removeFromTop(5); //separar 5 pixeles de arriba.
    
    analyzerEnabledButton.setBounds(analyzerEnabledArea); //Renderizamos el boton...
    
    bounds.removeFromBottom(5); //Creamos espacio para todo lo demás...
    
    float hRatio = 25.f/100.f;
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * hRatio);
    
    responseCurveComponent.setBounds(responseArea);
    
    bounds.removeFromTop(5);
    
    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth()*0.33);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth()*0.5);
    
    lowCutBypassButton.setBounds(lowCutArea.removeFromTop(25));
    lowcutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight()*0.5));
    lowcutSlopeSlider.setBounds(lowCutArea);
    
    highCutBypassButton.setBounds(highCutArea.removeFromTop(25));
    highcutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight()*0.5));
    highcutSlopeSlider.setBounds(highCutArea);
    
    peakBypassButton.setBounds(bounds.removeFromTop(25));
    peakFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight()*0.33));
    peakGainSlider.setBounds(bounds.removeFromTop(bounds.getHeight()*0.5));
    peakQualitySlider.setBounds(bounds);
    
}


std::vector<juce::Component*> EelEQAudioProcessorEditor::getComps(){
    
    return
    {
        &peakFreqSlider,
        &peakGainSlider,
        &peakQualitySlider,
        &lowcutFreqSlider,
        &highcutFreqSlider,
        &lowcutSlopeSlider,
        &highcutSlopeSlider,
        &responseCurveComponent,
        
        //Bypass buttons...
        &lowCutBypassButton,
        &highCutBypassButton,
        &peakBypassButton,
        &analyzerEnabledButton
        
    };
    
}


