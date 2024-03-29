/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
EelEQAudioProcessor::EelEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

EelEQAudioProcessor::~EelEQAudioProcessor()
{
}

//==============================================================================
const juce::String EelEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EelEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EelEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EelEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double EelEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EelEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int EelEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EelEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String EelEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void EelEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void EelEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    juce::dsp::ProcessSpec spec;
    
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;
    
    leftChain.prepare(spec);
    rightChain.prepare(spec);
    
    UpdateFilters();
    
    //preparar FIFOS
    leftChannelFifo.prepare(samplesPerBlock);
    rightChannelFifo.prepare(samplesPerBlock);
    
    //Preparar el oscilador auxiliar...
    osc.initialise([](float x){return std::sin(x);}); // esta función lambda pasa una sinoidal
    
    spec.numChannels = getTotalNumInputChannels();
    osc.prepare(spec);
    osc.setFrequency(100);
    
    
}

void EelEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EelEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void EelEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    
    UpdateFilters();
    
    // Definir la instancia del AudioBlock
    juce::dsp::AudioBlock<float> block(buffer);
    
    
    //Oscilador de calibración para la FFT: (por el momento está apagado)
    
//    buffer.clear();
//    juce::dsp::ProcessContextReplacing<float> stereoContext(block);
//    osc.process(stereoContext);
    
    
    
    
    // Extraer los canales dentro del AudioBlock
    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);
    
    // Crear el ProcessContext
    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
    
    // Pasar el contexto a las cadenas
    leftChain.process(leftContext);
    rightChain.process(rightContext);
    
    //Update to Fifo's
    leftChannelFifo.update(buffer);
    rightChannelFifo.update(buffer);
    
}

//==============================================================================
bool EelEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* EelEQAudioProcessor::createEditor()
{
    return new EelEQAudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void EelEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    // Guardar los parametros del bloque de datos...
    
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
    
    
}

void EelEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    
    if(tree.isValid()){
        
        apvts.replaceState(tree);
        UpdateFilters();
        
    }
    
}


//==============================================================================


ChainSettings getChainSettings (juce::AudioProcessorValueTreeState& apvts){
    
    ChainSettings settings;
    
    
    //HPF y LPF
    settings.lowCutFreq = apvts.getRawParameterValue("LowCut Freq")->load();
    settings.highCutFreq = apvts.getRawParameterValue("HighCut Freq")->load();
    settings.lowCutSlope = static_cast<Slope>(apvts.getRawParameterValue("LowCut Slope")->load());
    settings.highCutSlope = static_cast<Slope>(apvts.getRawParameterValue("HighCut Slope")->load());
    
    
    //Bell
    settings.peakFreq = apvts.getRawParameterValue("Peak Freq")->load();
    settings.peakGainInDecibels = apvts.getRawParameterValue("Peak Gain")->load();
    settings.peakQuality = apvts.getRawParameterValue("Quality")->load();
    
    //Bypass Settings
    settings.lowCutBypassed = apvts.getRawParameterValue("LowCut Bypassed")->load() > 0.5f;
    settings.highCutBypassed = apvts.getRawParameterValue("HighCut Bypassed")->load() > 0.5f;
    settings.peakBypassed = apvts.getRawParameterValue("Peak Bypassed")->load() > 0.5f;
    
    return settings;
}

Coefficients makePeakFilter(const ChainSettings& chainSettings, double sampleRate){
    
     return juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate,
                                                                chainSettings.peakFreq, chainSettings.peakQuality,
                                                                juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels));
    
}



//Definimos la función auxiliar que nos ayudara a actualizar los parametros del peak filter...
void EelEQAudioProcessor::UpdatePeakFilter(const ChainSettings &chainSettings){
    
   /* auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(),
                                                                                chainSettings.peakFreq, chainSettings.peakQuality,
                                                                                juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels)); */
    
    auto peakCoefficients = makePeakFilter(chainSettings, getSampleRate());
    
    leftChain.setBypassed<ChainPositions::Peak>(chainSettings.peakBypassed);
    rightChain.setBypassed<ChainPositions::Peak>(chainSettings.peakBypassed);
    
    UpdateCoefficients(leftChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
    UpdateCoefficients(rightChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
   
    
}


void UpdateCoefficients(Coefficients& old, const Coefficients &replacements){
    
    *old = *replacements;
    
}

void EelEQAudioProcessor::UpdateLowCut(const ChainSettings &chainSettings){
    
    //HighPass Filter

    auto cutCoefficients = makeLowCutFilter(chainSettings, getSampleRate());

    auto& leftLowCut = leftChain.get<ChainPositions::LowCut>();
    auto& rightLowCut = rightChain.get<ChainPositions::LowCut>();
    
    leftChain.setBypassed<ChainPositions::LowCut>(chainSettings.lowCutBypassed);
    rightChain.setBypassed<ChainPositions::LowCut>(chainSettings.lowCutBypassed);
    
    
    UpdateCutFilter(leftLowCut, cutCoefficients, chainSettings.lowCutSlope);
    UpdateCutFilter(rightLowCut, cutCoefficients, chainSettings.lowCutSlope);
    
    
}

void EelEQAudioProcessor::UpdateHighCut(const ChainSettings &chainSettings){
    
    // LowPass Filter
    
    auto highcutCoefficients = makeHighCutFilter(chainSettings, getSampleRate());
    
    auto& leftHighCut = leftChain.get<ChainPositions::HighCut>();
    auto& rightHighCut = rightChain.get<ChainPositions::HighCut>();
    
    leftChain.setBypassed<ChainPositions::HighCut>(chainSettings.highCutBypassed);
    rightChain.setBypassed<ChainPositions::HighCut>(chainSettings.highCutBypassed);
    
    UpdateCutFilter(leftHighCut, highcutCoefficients, chainSettings.highCutSlope);
    UpdateCutFilter(rightHighCut, highcutCoefficients, chainSettings.highCutSlope);
    
    
    
}

void EelEQAudioProcessor::UpdateFilters(){
    
    auto chainSettings = getChainSettings(apvts);
    
    UpdatePeakFilter(chainSettings);
    UpdateLowCut(chainSettings);
    UpdateHighCut(chainSettings);
    
    
}



juce::AudioProcessorValueTreeState::ParameterLayout EelEQAudioProcessor::createParameterLayout() {
    
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    //Float Parameters
    
    layout.add(
               std::make_unique<juce::AudioParameterFloat>("LowCut Freq",
                                                           "LowCut Freq",
                                                           juce::NormalisableRange<float>(20.f,20000.f,1.f,0.2f),
                                                           20.f
                                                           )
               );
    
    layout.add(
               std::make_unique<juce::AudioParameterFloat>("HighCut Freq",
                                                           "HighCut Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.20f),
                                                           20000.f
                                                           )
               );
    
    layout.add(
               std::make_unique<juce::AudioParameterFloat>("Peak Freq",
                                                           "Peak Freq",
                                                           juce::NormalisableRange<float>(20.f,20000.f,1.f,0.20f),
                                                           1000.f
                                                           )
               );
    
    layout.add(
               std::make_unique<juce::AudioParameterFloat>("Peak Gain",
                                                           "Peak Gain",
                                                           juce::NormalisableRange<float>(-24.f,24.f,0.1f,1.f),
                                                           0.f
                                                           )
               );
    
    layout.add(
               std::make_unique<juce::AudioParameterFloat>("Quality",
                                                           "Quality",
                                                           juce::NormalisableRange<float>(0.1f,15.f,0.05f,1.f),
                                                           0.9f
                                                           )
               );
    
    //Choice Parameters
    
    juce::StringArray stringArray;
    
    for ( int i = 0;  i < 4; i++ ){
        
        juce::String str;
        str << (12 + i*12);
        str << " dB/Oct";
        stringArray.add(str);
        
    }
    
    layout.add(
               std::make_unique<juce::AudioParameterChoice>("LowCut Slope",
                                                            "LowCut Slope",
                                                            stringArray,
                                                            0)
               );
    
    layout.add(
               std::make_unique<juce::AudioParameterChoice>("HighCut Slope",
                                                            "HighCut Slope",
                                                            stringArray,
                                                            0)
               );
    
    
    //Bypass Parameters...
    
    layout.add(std::make_unique<juce::AudioParameterBool>("LowCut Bypassed",
                                                          "LowCut Bypassed",
                                                          false));
    layout.add(std::make_unique<juce::AudioParameterBool>("HighCut Bypassed",
                                                          "HighCut Bypassed",
                                                          false));
    layout.add(std::make_unique<juce::AudioParameterBool>("Peak Bypassed",
                                                          "Peak Bypassed",
                                                          false));
    layout.add(std::make_unique<juce::AudioParameterBool>("Analyzer Enabled",
                                                          "Analyzer Enabled",
                                                          true));
    
    return layout;
}








//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EelEQAudioProcessor();
}
