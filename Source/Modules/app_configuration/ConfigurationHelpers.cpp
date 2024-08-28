#include "ConfigurationHelpers.h"
#include <yaml-cpp/yaml.h>

juce::File ConfigurationHelpers::SAVED_TRACK_NAME;

void ConfigurationHelpers::setSavedTrackName(const juce::File &newValue) {
    SAVED_TRACK_NAME = newValue;
}

juce::File ConfigurationHelpers::getSavedTrackName() {
    return SAVED_TRACK_NAME;
}

juce::String ConfigurationHelpers::getApplicationName() {
    return ROOT_DIRECTORY_NAME;
}

bool ConfigurationHelpers::writeBinarySamplesToDirectory(
    const juce::File &destDir, juce::StringRef filename, const char *data,
    int dataSizeInBytes) {
    auto f = destDir.getChildFile(filename);
    jassert(data != nullptr);
    return f.replaceWithData(data, static_cast<size_t>(dataSizeInBytes));
}

void ConfigurationHelpers::initBinarySamples(const juce::File &tempSynthDir,
                                             const juce::File &tempDrumDir) {
    //    NB: Binary samples are currently not used, so the sample data
    //    libraries do not get built which is why this is commented out for (int
    //    i = 0; i < SynthSampleData::namedResourceListSize; ++i) {
    //        int dataSizeInBytes = 0;
    //        const char *data = SynthSampleData::getNamedResource(
    //            SynthSampleData::namedResourceList[i], dataSizeInBytes);
    //        auto success = writeBinarySampleToDirectory(
    //            tempSynthDir, SynthSampleData::originalFilenames[i], data,
    //            dataSizeInBytes);
    //        if (!success) {
    //            juce::Logger::writeToLog(
    //                "Attempt to write binary synth sample data file
    //                failed!");
    //        }
    //    }
    //
    //    for (int i = 0; i < DrumSampleData::namedResourceListSize; ++i) {
    //        int dataSizeInBytes = 0;
    //        const char *data = DrumSampleData::getNamedResource(
    //            DrumSampleData::namedResourceList[i], dataSizeInBytes);
    //        auto success = writeBinarySampleToDirectory(
    //            tempDrumDir, DrumSampleData::originalFilenames[i], data,
    //            dataSizeInBytes);
    //        if (!success) {
    //            juce::Logger::writeToLog(
    //                "Attempt to write binary drum sample data file
    //                failed!");
    //        }
    //    }
}

void ConfigurationHelpers::initUserSamples(const juce::File &userSynthSampleDir,
                                           const juce::File &userDrumDir,
                                           const juce::File &tempSynthDir,
                                           const juce::File &tempDrumDir) {
    auto userAppDataDirectory = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory);

    if (userSynthSampleDir.exists()) {
        auto success = userSynthSampleDir.copyDirectoryTo(tempSynthDir);
        if (!success)
            juce::Logger::writeToLog(
                "Attempt to copy user synth sample data failed!");
        else
            juce::Logger::writeToLog("User samples copied");
    } else {
        juce::Logger::writeToLog(
            "User synth sample directory does not exist, creating it now.");
        auto result = userSynthSampleDir.createDirectory();
        if (result.failed())
            juce::Logger::writeToLog(
                "Attempt to create user synth sample directory failed!: " +
                result.getErrorMessage());
    }

    if (userDrumDir.exists()) {
        auto success = userDrumDir.copyDirectoryTo(tempDrumDir);
        if (!success)
            juce::Logger::writeToLog(
                "Attempt to copy user drum kit data failed!");
        else
            juce::Logger::writeToLog("User drum kits copied");
    } else {
        juce::Logger::writeToLog(
            "User drum kit directory does not exist, creating it now.");
        auto result = userDrumDir.createDirectory();
        if (result.failed())
            juce::Logger::writeToLog(
                "Attempt to create user drum kit directory failed!: " +
                result.getErrorMessage());
    }
}
juce::File
ConfigurationHelpers::createTempDirectory(tracktion::Engine &engine,
                                          const juce::String &folderName) {
    auto dir = engine.getTemporaryFileManager().getTempFile(folderName);
    auto result = dir.createDirectory();
    if (!result.wasOk()) {
        juce::Logger::writeToLog("Error creating temporary directory " +
                                 folderName);
        return {};
    } else {
        return dir;
    }
}

void ConfigurationHelpers::initSamples(tracktion::Engine &engine) {
    auto userAppDataDirectory = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory);
    auto userSynthSampleDir = getSamplesDirectory();
    auto userDrumDir = getDrumKitsDirectory();
    auto tempSynthSamplesDir =
        createTempDirectory(engine, SAMPLES_DIRECTORY_NAME);
    auto tempDrumKitsDir =
        createTempDirectory(engine, DRUM_KITS_DIRECTORY_NAME);
    //        initBinarySamples(tempSynthSamplesDir, tempDrumKitsDir);
    initUserSamples(userSynthSampleDir, userDrumDir, tempSynthSamplesDir,
                    tempDrumKitsDir);
}

bool ConfigurationHelpers::getShowTitleBar(juce::File &configFile) {
    if (configFile.exists()) {
        YAML::Node rootNode =
            YAML::LoadFile(configFile.getFullPathName().toStdString());
        YAML::Node config = rootNode["config"];
        if (config)
            if (config["show-title-bar"])
                return config["show-title-bar"].as<bool>();
    }

    // Default to showing the title bar
    return true;
}

double ConfigurationHelpers::getWidth(juce::File &configFile) {
    if (configFile.exists()) {
        YAML::Node rootNode =
            YAML::LoadFile(configFile.getFullPathName().toStdString());
        YAML::Node config = rootNode["config"];
        if (config) {
            if (config["size"]) {
                auto sizeConfig = config["size"];
                if (sizeConfig["width"])
                    return sizeConfig["width"].as<double>();
            }
        }
    }

    // Default to 800
    return 800;
}

double ConfigurationHelpers::getHeight(juce::File &configFile) {
    if (configFile.exists()) {
        YAML::Node rootNode =
            YAML::LoadFile(configFile.getFullPathName().toStdString());
        YAML::Node config = rootNode["config"];
        if (config) {
            if (config["size"]) {
                auto sizeConfig = config["size"];
                if (sizeConfig["height"])
                    return sizeConfig["height"].as<double>();
            }
        }
    }

    // Default to 480
    return 480;
}

juce::File ConfigurationHelpers::getSamplesDirectory() {
    auto userAppDataDirectory = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory);
    return userAppDataDirectory.getChildFile(ROOT_DIRECTORY_NAME)
        .getChildFile(SAMPLES_DIRECTORY_NAME);
}

juce::File ConfigurationHelpers::getDrumKitsDirectory() {
    auto userAppDataDirectory = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory);
    return userAppDataDirectory.getChildFile(ROOT_DIRECTORY_NAME)
        .getChildFile(DRUM_KITS_DIRECTORY_NAME);
}

juce::File
ConfigurationHelpers::getTempSamplesDirectory(tracktion::Engine &engine) {
    return engine.getTemporaryFileManager().getTempFile(SAMPLES_DIRECTORY_NAME);
}

juce::File
ConfigurationHelpers::getTempDrumKitsDirectory(tracktion::Engine &engine) {
    return engine.getTemporaryFileManager().getTempFile(
        DRUM_KITS_DIRECTORY_NAME);
}
