// clang-format off
/*******************************************************************************
 BEGIN_JUCE_MODULE_DECLARATION
  ID:               app_view_models
  vendor:           Stone Preston
  version:          1.0.0
  name:             App View Models
  description:      View model classes for app
  website:          http://github.com/stonepreston
  license:          GPL-3.0
  dependencies:     juce_data_structures tracktion_engine juce_events juce_core juce_graphics internal_plugins app_models app_services internal_plugins
 END_JUCE_MODULE_DECLARATION
*******************************************************************************/
#pragma once

namespace app_view_models {
    class FlaggedAsyncUpdater;
    class MidiCommandManager;
    class ItemListState;
    class EditItemListViewModel;
    class ModifierList;
    class EditItemListAdapter;
    class MixerTracksListAdapter;
    class TracksListAdapter;
    class PluginsListAdapter;
    class ModifiersListAdapter;
    class ModifierPluginDestinationsListAdapter;
    class ModifierViewModel;
    class LFOModifierViewModel;
    class EditItemListViewModel;
    class TracksListViewModel;
    class TrackViewModel;
    class TrackPluginsListViewModel;
    class AvailablePluginsViewModel;
    class InternalPluginViewModel;
    class ReverbPluginViewModel;
    class DelayPluginViewModel;
    class LowPassPluginViewModel;
    class PhaserPluginViewModel;
    class ChorusPluginViewModel;
    class EqualiserPluginViewModel;
    class CompressorPluginViewModel;
    class AvailableSequencersListViewModel;
    class StepSequencerViewModel;
    class SamplerViewModel;
    class SynthSamplerViewModel;
    class DrumSamplerViewModel;
    class FourOscPluginViewModel;
    class OscillatorViewModel;
    class ADSRViewModel;
    class FilterViewModel;
    class MixerViewModel;
    class MixerTrackViewModel;
    class SettingsListViewModel;
    class DeviceTypeListViewModel;
    class LoadSaveSongListViewModel;
    class OutputListViewModel;
    class SampleRateListViewModel;
    class AudioBufferSizeListViewModel;
    class MidiInputListViewModel;
    class EditViewModel;
}

#include <juce_data_structures/juce_data_structures.h>
#include <juce_events/juce_events.h>
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <tracktion_engine/tracktion_engine.h>
#include <app_models/app_models.h>
#include <app_services/app_services.h>
#include <internal_plugins/internal_plugins.h>
#include <functional>
#include <app_configuration/app_configuration.h>

// Utilities
#include "Utilities/FlaggedAsyncUpdater.h"
#include "Utilities/EngineHelpers.h"

// ItemList
#include "Edit/ItemList/ItemListState.h"
#include "Edit/ItemList/ListAdapters/EditItemListAdapter.h"
#include "Edit/ItemList/ListAdapters/MixerTracksListAdapter.h"
#include "Edit/ItemList/ListAdapters/TracksListAdapter.h"
#include "Edit/ItemList/ListAdapters/PluginsListAdapter.h"
#include "Edit/Modifiers/ModifierList.h"
#include "Edit/ItemList/ListAdapters/ModifiersListAdapter.h"
#include "Edit/ItemList/ListAdapters/ModifierPluginDestinationsListAdapter.h"
#include "Edit/ItemList/EditItemListViewModel.h"

// Tracks
#include "Edit/Tracks/TrackViewModel.h"
#include "Edit/Tracks/TracksListViewModel.h"

// Plugins
#include "Edit/Plugins/PluginTree/PluginTreeBase.h"
#include "Edit/Plugins/PluginTree/PluginTreeGroup.h"
#include "Edit/Plugins/PluginTree/PluginTreeItem.h"
#include "Edit/Plugins/TrackPluginsListViewModel.h"
#include "Edit/Plugins/AvailablePluginsViewModel.h"
#include "Edit/Plugins/Sampler/SamplerViewModel.h"
#include "Edit/Plugins/Sampler/SynthSamplerViewModel.h"
#include "Edit/Plugins/Sampler/DrumSamplerViewModel.h"
#include "Edit/Plugins/InternalPluginViewModel.h"
#include "Edit/Plugins/ReverbPluginViewModel.h"
#include "Edit/Plugins/DelayPluginViewModel.h"
#include "Edit/Plugins/LowPassPluginViewModel.h"
#include "Edit/Plugins/PhaserPluginViewModel.h"
#include "Edit/Plugins/ChorusPluginViewModel.h"
#include "Edit/Plugins/EqualiserPluginViewModel.h"
#include "Edit/Plugins/CompressorPluginViewModel.h"
#include "Edit/Plugins/FourOscPluginViewModel.h"
#include "Edit/Plugins/FourOsc/OscillatorViewModel.h"
#include "Edit/Plugins/FourOsc/ADSRViewModel.h"
#include "Edit/Plugins/FourOsc/FilterViewModel.h"

// Modifiers
#include "Edit/Modifiers/TrackModifiersListViewModel.h"
#include "Edit/Modifiers/AvailableModifiersListViewModel.h"
#include "Edit/Modifiers/AvailablePluginParametersListViewModel.h"
#include "Edit/Modifiers/ModifierPluginDestinationsViewModel.h"
#include "Edit/Modifiers/ModifierViewModel.h"
#include "Edit/Modifiers/LFOModifierViewModel.h"

// Tempo
#include "Edit/Tempo/TempoSettingsViewModel.h"

// Sequencers
#include "Edit/Sequencers/AvailableSequencersListViewModel.h"
#include "Edit/Sequencers/StepSequencerViewModel.h"

// Mixer
#include "Edit/Mixer/MixerViewModel.h"
#include "Edit/Mixer/MixerTrackViewModel.h"

// Settings
#include "Edit/Settings/SettingsListViewModel.h"
#include "Edit/Settings/DeviceTypeListViewModel.h"
#include "Edit/Settings/LoadSaveSongListViewModel.h"
#include "Edit/Settings/OutputListViewModel.h"
#include "Edit/Settings/SampleRateListViewModel.h"
#include "Edit/Settings/AudioBufferSizeListViewModel.h"
#include "Edit/Settings/MidiInputListViewModel.h"

// Edit
#include "Edit/EditViewModel.h"