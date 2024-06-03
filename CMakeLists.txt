cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(LMN-3 VERSION 0.1.1)

add_subdirectory(yaml-cpp)
add_subdirectory(tracktion_engine/modules/juce)
add_subdirectory(Plugins)

option(PACKAGE_TESTS "Build the tests" ON)
if(PACKAGE_TESTS)
    enable_testing()
    include(GoogleTest)
    add_subdirectory(Tests)
endif()

juce_add_gui_app(LMN-3
    # VERSION ...                       # Set this if the app version is different to the project version
    # ICON_BIG ...                      # ICON_* arguments specify a path to an image file to use as an icon
    # ICON_SMALL ...
    # DOCUMENT_EXTENSIONS ...           # Specify file extensions that should be associated with this app
    # COMPANY_NAME ...                  # Specify the name of the app's author
    PRODUCT_NAME LMN-3         # The name of the final executable, which can differ from the target name
)

target_compile_features(LMN-3  PRIVATE cxx_std_17)

target_sources(LMN-3  PRIVATE
    Source/Main.cpp
    Source/Views/App/App.cpp
    Source/Views/App/MessageBox.cpp
    Source/Views/App/ControlButtonIndicator.cpp
    Source/Views/App/ProgressView/ProgressView.cpp
    Source/Views/App/ProgressView/SVGImageComponent.cpp
    Source/Views/LookAndFeel/AppLookAndFeel.cpp
    Source/Views/LookAndFeel/Labels/LabelColour1LookAndFeel.cpp
    Source/Views/LookAndFeel/ListItems/ListItemColour2LookAndFeel.cpp
    Source/Views/Edit/Settings/SettingsListView.cpp
    Source/Views/Edit/Settings/DeviceTypeListView.cpp
    Source/Views/Edit/Settings/LoadSaveSongListView.cpp
    Source/Views/Edit/Settings/OutputListView.cpp
    Source/Views/Edit/Settings/SampleRateListView.cpp
    Source/Views/Edit/Settings/MidiInputListView.cpp
    Source/Views/Edit/Settings/AudioBufferSizeListView.cpp
    Source/Views/SimpleList/SimpleListItemView.cpp
    Source/Views/SimpleList/SimpleListModel.cpp
    Source/Views/SimpleList/SimpleListView.cpp
    Source/Views/SimpleList/SplitListView.cpp
    Source/Views/SimpleList/ListTitle.cpp
    Source/Views/SimpleList/TitledListView.cpp
    Source/Views/SimpleList/TitledSplitListView.cpp
    Source/Views/Knobs/LabeledKnob.cpp
    Source/Views/Edit/EditTabBarView.cpp
    Source/Views/Edit/OctaveDisplayComponent.cpp
    Source/Views/Edit/Tempo/TempoSettingsView.cpp
    Source/Views/Edit/Tempo/BeatSettingsComponent.cpp
    Source/Views/Knobs/Knobs.cpp
    Source/Views/Edit/Plugins/TrackPluginsListView.cpp
    Source/Views/Edit/Plugins/ExtendedUIBehaviour.h
    Source/Views/Edit/Plugins/PluginView.cpp
    Source/Views/Edit/Plugins/AvailablePluginsListView.cpp
    Source/Views/Edit/Plugins/Sampler/SamplerView.cpp
    Source/Views/Edit/Plugins/Sampler/ThumbnailComponent.cpp
    Source/Views/Edit/Plugins/Sampler/ThumbnailMarkerComponent.cpp
    Source/Views/Edit/Plugins/InternalPlugin/InternalPluginView.cpp
    Source/Views/Edit/Plugins/LowPassPluginView.cpp
    Source/Views/Edit/Plugins/FourOsc/FourOscView.cpp
    Source/Views/Edit/Plugins/FourOsc/OscillatorView.cpp
    Source/Views/Edit/Plugins/FourOsc/ADSRView.cpp
    Source/Views/Edit/Plugins/FourOsc/ADSRPlot.cpp
    Source/Views/Edit/Plugins/FourOsc/FilterADSRView.cpp
    Source/Views/Edit/Plugins/FourOsc/FilterView.cpp
    Source/Views/Edit/Modifiers/TrackModifiersListView.cpp
    Source/Views/Edit/Modifiers/AvailableModifiersListView.cpp
    Source/Views/Edit/Modifiers/ModifierPluginDestinationsListView.cpp
    Source/Views/Edit/Modifiers/AvailablePluginParametersListView.cpp
    Source/Views/Edit/Modifiers/ModifierView.cpp
    Source/Views/Edit/Tracks/InformationPanelComponent.cpp
    Source/Views/Edit/Tracks/Markers/PlayheadComponent.cpp
    Source/Views/Edit/Tracks/Markers/LoopMarkerComponent.cpp
    Source/Views/Edit/Tracks/TracksView.cpp
    Source/Views/Edit/Tracks/TracksListBoxModel.cpp
    Source/Views/Edit/Tracks/Track/TrackView.cpp
    Source/Views/Edit/Tracks/Track/SelectedTrackMarker.cpp
    Source/Views/Edit/Tracks/Track/Clips/ClipComponent.cpp
    Source/Views/Edit/Tracks/Track/Clips/MidiClipComponent.cpp
    Source/Views/Edit/Tracks/Track/Clips/RecordingClipComponent.cpp
    Source/Views/Edit/Sequencers/AvailableSequencersListView.cpp
    Source/Views/Edit/Sequencers/StepSequencer/StepSequencerView.cpp
    Source/Views/Edit/Sequencers/StepSequencer/StepSequencerGridComponent.cpp
    Source/Views/Edit/Mixer/MixerView.cpp
    Source/Views/Edit/Mixer/MixerTableListBoxModel.cpp
    Source/Views/Edit/Mixer/MixerTrackView.cpp
    Source/Views/Edit/Mixer/LevelMeterComponent.cpp
)

target_include_directories(LMN-3  PUBLIC
    Source/
    Source/Views
    Source/Views/App
    Source/Views/App/ProgressView
    Source/Views/LookAndFeel
    Source/Views/LookAndFeel/Labels
    Source/Views/LookAndFeel/ListItems
    Source/Views/Edit/Settings
    Source/Views/Utilities
    Source/Views/SimpleList
    Source/Views/Knobs
    Source/Views/Edit
    Source/Views/Edit/Tempo
    Source/Views/Edit/CurrentTrack
    Source/Views/Edit/Plugins
    Source/Views/Edit/Plugins/Sampler
    Source/Views/Edit/Plugins/FourOsc
    Source/Views/Edit/Plugins/InternalPlugin
    Source/Views/Edit/Modifiers
    Source/Views/Edit/Tracks
    Source/Views/Edit/Tracks/Markers
    Source/Views/Edit/Tracks/Track
    Source/Views/Edit/Tracks/Track/Clips
    Source/Views/Edit/Sequencers
    Source/Views/Edit/Sequencers/StepSequencer
    Source/Views/Edit/Mixer
    Source/Views/Utilities
)

target_compile_definitions(LMN-3  PRIVATE
    JUCE_MODAL_LOOPS_PERMITTED=1 # For Tracktion Engine
    JUCE_PLUGINHOST_VST3=1
    JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:LMN-3,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:LMN-3,JUCE_VERSION>"
)

# add binary data
juce_add_binary_data(ImageData HEADER_NAME ImageData.h NAMESPACE ImageData SOURCES
    tracktion_engine/tutorials/images/tracktion_engine_powered.png
    Assets/Images/lemon-solid.svg
)

juce_add_binary_data(FontData HEADER_NAME FontData.h NAMESPACE FontData SOURCES
    FontAwesome/FontAwesome6BrandsRegular400.otf
    FontAwesome/FontAwesome6FreeSolid900.otf
    FontAwesome/FontAwesome6FreeRegular400.otf
)

# If at some point samples are ever built directly into the application, they will need to be added below
#juce_add_binary_data(SynthSampleData HEADER_NAME SynthSampleData.h NAMESPACE SynthSampleData SOURCES
#)

#juce_add_binary_data(DrumSampleData HEADER_NAME DrumSampleData.h NAMESPACE DrumSampleData SOURCES
#)

# add modules
juce_add_module(tracktion_engine/modules/tracktion_engine)
juce_add_module(tracktion_engine/modules/tracktion_graph)
juce_add_module(Source/Modules/app_navigation)
juce_add_module(Source/Modules/app_services)
juce_add_module(Source/Modules/app_models)
juce_add_module(Source/Modules/internal_plugins)
juce_add_module(Source/Modules/app_view_models)
juce_add_module(Source/Modules/app_configuration)

# link sample data to app models
#target_link_libraries(app_view_models INTERFACE SynthSampleData)
#target_link_libraries(app_view_models INTERFACE DrumSampleData)

target_link_libraries(LMN-3
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        tracktion_engine
        tracktion_graph
        app_navigation
        app_services
        app_models
        internal_plugins
        app_view_models
        app_configuration
        ImageData
        FontData
    #    SynthSampleData
    #    DrumSampleData
        atomic
        yaml-cpp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
