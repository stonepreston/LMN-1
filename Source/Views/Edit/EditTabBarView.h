#pragma once
#include "MessageBox.h"
#include "OctaveDisplayComponent.h"
#include <app_navigation/app_navigation.h>
#include <app_services/app_services.h>
#include <app_view_models/app_view_models.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <tracktion_engine/tracktion_engine.h>

class EditTabBarView : public juce::TabbedComponent,
                       public app_services::MidiCommandManager::Listener,
                       public app_view_models::ItemListState::Listener,
                       public app_view_models::EditViewModel::Listener,
                       juce::Timer {
  public:
    EditTabBarView(tracktion::Edit &e, app_services::MidiCommandManager &mcm);
    ~EditTabBarView() override;
    void paint(juce::Graphics &) override;
    void resized() override;
    void tracksButtonReleased() override;
    void tempoSettingsButtonReleased() override;
    void saveButtonReleased() override;
    void renderButtonReleased() override;
    void mixerButtonReleased() override;
    void settingsButtonReleased() override;
    void pluginsButtonReleased() override;
    void modifiersButtonReleased() override;
    void sequencersButtonReleased() override;
    void currentTabChanged(int newCurrentTabIndex,
                           const juce::String &newCurrentTabName) override;

    // MidiCommandManager Listener
    void octaveChanged(int newOctave) override;

    // Used to reset the modifiers list when ever a plugin gets deleted
    void resetModifiersTab();

    // ItemListState listener methods
    void selectedIndexChanged(int newIndex) override;

    // ViewModel listener
    void trackDeleted() override;

  private:
    tracktion::Edit &edit;
    app_services::MidiCommandManager &midiCommandManager;
    app_view_models::EditViewModel viewModel;
    juce::String tracksTabName = "TRACKS";
    juce::String tempoSettingsTabName = "TEMPO_SETTINGS";
    juce::String mixerTabName = "MIXER";
    juce::String pluginsTabName = "PLUGINS";
    juce::String modifiersTabName = "MODIFIERS";
    juce::String sequencersTabName = "SEQUENCERS";
    juce::String settingsTabName = "SETTINGS";
    juce::String applicationName = JUCE_APPLICATION_NAME_STRING;

    OctaveDisplayComponent octaveDisplayComponent;
    MessageBox messageBox;

    void timerCallback() override;
    void resetTrackRelatedTabs();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EditTabBarView)
};
