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
    

//    auto chainSettings = getChainSettings(apvts);
//
//
//    UpdatePeakFilter(chainSettings);
//
//    //HighPass Filter: Left Channel
//
//    auto lowcutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq, sampleRate, 2*(chainSettings.lowCutSlope + 1));
//
//    auto& leftLowCut = leftChain.get<ChainPositions::LowCut>();
//    UpdateCutFilter(leftLowCut, lowcutCoefficients, chainSettings.lowCutSlope);
//
//    //HPF: Right Channel
//    auto& rightLowCut = rightChain.get<ChainPositions::LowCut>();
//    UpdateCutFilter(rightLowCut, lowcutCoefficients, chainSettings.lowCutSlope);
//
//
//
//    // LowPass Filter: Left Channel
//
//    auto highcutCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.highCutFreq, sampleRate, 2*(chainSettings.highCutSlope +1));
//
//    auto& leftHighCut = leftChain.get<ChainPositions::HighCut>();
//    UpdateCutFilter(leftHighCut, highcutCoefficients, chainSettings.highCutSlope);
//
//    // LPF: Right Channel
//    auto& rightHighCut = rightChain.get<ChainPositions::HighCut>();
//    UpdateCutFilter(rightHighCut, highcutCoefficients, chainSettings.highCutSlope);
//
    UpdateFilters();
    
    
    
    
    
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
    
    
    // Update Coefficients...
    
//    auto chainSettings = getChainSettings(apvts);
//
//    // Update PeakFilter
//    UpdatePeakFilter(chainSettings);
//
//
//
//    //HighPass Filter: Left Channel
//
//    auto cutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq, getSampleRate(), 2*(chainSettings.lowCutSlope + 1));
//
//    auto& leftLowCut = leftChain.get<ChainPositions::LowCut>();
//
//    UpdateCutFilter(leftLowCut, cutCoefficients, chainSettings.lowCutSlope);
//
//
//    // HPF: Right Channel
//
//    auto& rightLowCut = rightChain.get<ChainPositions::LowCut>();
//    UpdateCutFilter(rightLowCut, cutCoefficients, chainSettings.lowCutSlope);
//
//
//
//    // LowPass Filter: Left Channel
//
//    auto highcutCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.highCutFreq, getSampleRate(), 2*(chainSettings.highCutSlope +1));
//
//    auto& leftHighCut = leftChain.get<ChainPositions::HighCut>();
//    UpdateCutFilter(leftHighCut, highcutCoefficients, chainSettings.highCutSlope);
//
//    // LPF: Right Channel
//    auto& rightHighCut = rightChain.get<ChainPositions::HighCut>();
//    UpdateCutFilter(rightHighCut, highcutCoefficients, chainSettings.highCutSlope);
//
    
    UpdateFilters();
    
    // Definir la instancia del AudioBlock
    juce::dsp::AudioBlock<float> block(buffer);
    
    // Extraer los canales dentro del AudioBlock
    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);
    
    // Crear el ProcessContext
    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
    
    // Pasar el contexto a las cadenas
    leftChain.process(leftContext);
    rightChain.process(rightContext);
    
    
    
    
    
}

//==============================================================================
bool EelEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* EelEQAudioProcessor::createEditor()
{
    //return new EelEQAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void EelEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void EelEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
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
    
    return settings;
}



//Definimos la función auxiliar que nos ayudara a actualizar los parametros del peak filter...
void EelEQAudioProcessor::UpdatePeakFilter(const ChainSettings &chainSettings){
    
    auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(),
                                                                                chainSettings.peakFreq, chainSettings.peakQuality,
                                                                                juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels));
    
    
    UpdateCoefficients(leftChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
    UpdateCoefficients(rightChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
    //*leftChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;
    //*rightChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;
    
}


void EelEQAudioProcessor::UpdateCoefficients(Coefficients& old, const Coefficients &replacements){
    
    *old = *replacements;
    
}

void EelEQAudioProcessor::UpdateLowCut(const ChainSettings &chainSettings){
    
    //HighPass Filter

    auto cutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq, getSampleRate(), 2*(chainSettings.lowCutSlope + 1));

    auto& leftLowCut = leftChain.get<ChainPositions::LowCut>();
    auto& rightLowCut = rightChain.get<ChainPositions::LowCut>();
    
    UpdateCutFilter(leftLowCut, cutCoefficients, chainSettings.lowCutSlope);
    UpdateCutFilter(rightLowCut, cutCoefficients, chainSettings.lowCutSlope);
    
    
}

void EelEQAudioProcessor::UpdateHighCut(const ChainSettings &chainSettings){
    
    // LowPass Filter
    
    auto highcutCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.highCutFreq, getSampleRate(), 2*(chainSettings.highCutSlope +1));
    
    auto& leftHighCut = leftChain.get<ChainPositions::HighCut>();
    auto& rightHighCut = rightChain.get<ChainPositions::HighCut>();
    
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
                                                           juce::NormalisableRange<float>(20.f,20000.f,1.f,0.25f),
                                                           20.f
                                                           )
               );
    
    layout.add(
               std::make_unique<juce::AudioParameterFloat>("HighCut Freq",
                                                           "HighCut Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
                                                           20000.f
                                                           )
               );
    
    layout.add(
               std::make_unique<juce::AudioParameterFloat>("Peak Freq",
                                                           "Peak Freq",
                                                           juce::NormalisableRange<float>(20.f,20000.f,1.f,0.25f),
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
        str << " dB/octv";
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
    
    return layout;
}








//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EelEQAudioProcessor();
}
