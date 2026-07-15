#include <juce_audio_processors/juce_audio_processors.h>

namespace juce
{
AudioProcessor* createPluginFilter()
{
    return new juce::AudioProcessor();
}
}
