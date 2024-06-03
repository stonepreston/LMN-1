#pragma once
#include <juce_core/juce_core.h>
#include <tracktion_engine/tracktion_engine.h>

class ConfigurationHelpers {
  public:
    static inline const juce::String ROOT_DIRECTORY_NAME = "LMN-3";
    static inline const juce::String SAMPLES_DIRECTORY_NAME = "samples";
    static inline const juce::String DRUM_KITS_DIRECTORY_NAME = "drum_kits";
    static juce::File SAVED_TRACK_NAME;

 
    static juce::File getSamplesDirectory();
    static juce::File getDrumKitsDirectory();
    static juce::File getTempSamplesDirectory(tracktion::Engine &engine);
    static juce::File getTempDrumKitsDirectory(tracktion::Engine &engine);
    static void initSamples(tracktion::Engine &engine);
    static bool getShowTitleBar(juce::File &configFile);
    static double getWidth(juce::File &configFile);
    static double getHeight(juce::File &configFile);    
    static void setSavedTrackName(const juce::File &newValue); // Declaración del método
    static juce::File getSavedTrackName();         // Declaración del método
    static juce::String getApplicationName(); // Declaración del método

  private:
      static bool writeBinarySamplesToDirectory(const juce::File &destDir,
                                              juce::StringRef filename,
                                              const char *data,
                                              int dataSizeInBytes);

    static void initBinarySamples(const juce::File &tempSynthDir,
                                  const juce::File &tempDrumDir);
    static void initUserSamples(const juce::File &userSynthSampleDir,
                                const juce::File &userDrumDir,
                                const juce::File &tempSynthDir,
                                const juce::File &tempDrumDir);
    static juce::File createTempDirectory(tracktion::Engine &engine,
                                          const juce::String &folderName);

};
