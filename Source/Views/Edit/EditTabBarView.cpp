#include "EditTabBarView.h"
#include "AvailableSequencersListView.h"
#include "FourOscView.h"
#include "MixerView.h"
#include "PluginView.h"
#include "SettingsListView.h"
#include "TempoSettingsView.h"
#include "TrackModifiersListView.h"
#include "TrackPluginsListView.h"
#include "TracksView.h"
EditTabBarView::EditTabBarView(tracktion::Edit &e,
                               app_services::MidiCommandManager &mcm)
    : TabbedComponent(juce::TabbedButtonBar::Orientation::TabsAtTop), edit(e),
      midiCommandManager(mcm), viewModel(edit) {
    // Note: Some tabs are on a per-track basis and are added in
    // selectedIndexChanged, not here this is possible since this view is a
    // listener of the tracks item list state
    addTab(tracksTabName, juce::Colours::transparentBlack,
           new TracksView(edit, midiCommandManager), true);
    addTab(tempoSettingsTabName, juce::Colours::transparentBlack,
           new TempoSettingsView(edit, midiCommandManager), true);
    addTab(mixerTabName, juce::Colours::transparentBlack,
           new MixerView(edit, midiCommandManager), true);
    addTab(settingsTabName, juce::Colours::transparentBlack,
           new app_navigation::StackNavigationController(new SettingsListView(
               edit, edit.engine.getDeviceManager().deviceManager,
               midiCommandManager)),
           true);

    juce::StringArray tabNames = getTabNames();
    int tracksIndex = tabNames.indexOf(tracksTabName);
    if (auto tracksView =
            dynamic_cast<TracksView *>(getTabContentComponent(tracksIndex))) {
        // this is so we can be notified when selected track changes
        tracksView->getViewModel().listViewModel.itemListState.addListener(
            this);
    }

    // hide tab bar
    setTabBarDepth(0);

    addChildComponent(octaveDisplayComponent);
    octaveDisplayComponent.setAlwaysOnTop(true);

    addChildComponent(messageBox);
    messageBox.setAlwaysOnTop(true);

    midiCommandManager.addListener(this);
    viewModel.addListener(this);

    // Set tracks as initial view
    setCurrentTabIndex(tracksIndex);
}

EditTabBarView::~EditTabBarView() {
    midiCommandManager.removeListener(this);
    viewModel.removeListener(this);
    juce::StringArray tabNames = getTabNames();
    int tracksIndex = tabNames.indexOf(tracksTabName);

    if (auto tracksView =
            dynamic_cast<TracksView *>(getTabContentComponent(tracksIndex)))
        tracksView->getViewModel().listViewModel.itemListState.removeListener(
            this);
}

void EditTabBarView::paint(juce::Graphics &g) {
    g.fillAll(
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void EditTabBarView::resized() {
    juce::TabbedComponent::resized();

    int octaveDisplayWidth = getWidth() / 6;
    int octaveDisplayHeight = getHeight() / 8;
    octaveDisplayComponent.setBounds((getWidth() - octaveDisplayWidth) / 2,
                                     (getHeight() - octaveDisplayHeight) / 2,
                                     octaveDisplayWidth, octaveDisplayHeight);

    auto font = messageBox.getFont();
    int width = font.getStringWidth(messageBox.getMessage());
    int messageBoxWidth = width + 50;
    int messageBoxHeight = getHeight() / 6;

    messageBox.setBounds((getWidth() - messageBoxWidth) / 2,
                         (getHeight() - messageBoxHeight) / 2, messageBoxWidth,
                         messageBoxHeight);
}

void EditTabBarView::tracksButtonReleased() {
    if (isShowing()) {
        juce::StringArray tabNames = getTabNames();
        int index = tabNames.indexOf(tracksTabName);
        if (index != getCurrentTabIndex()) {
            setCurrentTabIndex(index);
            // Clear the history so that only changes that happen on the tracks
            // tab can be undone ie dont let plugins that were added in the
            // plugins tab get removed or whatever
            edit.getUndoManager().clearUndoHistory();
            midiCommandManager.setFocusedComponent(
                getCurrentContentComponent());
        }
    }
}

void EditTabBarView::tempoSettingsButtonReleased() {
    if (isShowing()) {
        juce::StringArray tabNames = getTabNames();
        int index = tabNames.indexOf(tempoSettingsTabName);
        if (index != getCurrentTabIndex()) {
            setCurrentTabIndex(index);
            midiCommandManager.setFocusedComponent(
                getCurrentContentComponent());
        }
    }
}

/* void EditTabBarView::saveButtonReleased() {
    if (isShowing()) {
        juce::Logger::writeToLog("Saving edit ...");
        tracktion::EditFileOperations fileOperations(edit);
        fileOperations.save(true, true, false);
        juce::Logger::writeToLog("Save complete!");

        messageBox.setMessage("Save Complete!");
        // must call resized so message box width is updated to fit text
        resized();
        messageBox.setVisible(true);
        startTimer(1000);
    }
}*/


void EditTabBarView::saveButtonReleased() {      

    const auto track_name = ConfigurationHelpers::getSavedTrackName();
    
    //editFile.create();
    tracktion::EditFileOperations fileOperations(edit);
    auto userAppDataDirectory = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory);

    juce::File savedDirectory =
        userAppDataDirectory.getChildFile(JUCE_APPLICATION_NAME_STRING)
            .getChildFile("saved");


     if (!savedDirectory.exists()) {
        if (!savedDirectory.createDirectory()) {
            juce::Logger::writeToLog("Error creating folder: " +
                                     savedDirectory.getFullPathName());
            return;
        }
    }

     if (track_name.exists()) {
        fileOperations.saveAs(track_name, true);
    }    
    auto saveFile = savedDirectory.getChildFile(track_name.getFileName());      
    
    if (track_name.copyFileTo(saveFile)) {
        juce::Logger::writeToLog("Track copied to: " +
                                 saveFile.getFullPathName());
    } else {
        juce::Logger::writeToLog("Error copying to: " +
                                 saveFile.getFullPathName());
    }

    juce::Logger::writeToLog("Complete! (" + track_name.getFileNameWithoutExtension() + ")");
    messageBox.setMessage("Complete! (" + track_name.getFileNameWithoutExtension() + ")");
    resized();
    messageBox.setVisible(true);
    startTimer(1000);
}

void EditTabBarView::renderButtonReleased() {
    if (isShowing()) {
        juce::Logger::writeToLog("Rendering edit ...");
        auto userAppDataDirectory = juce::File::getSpecialLocation(
            juce::File::userApplicationDataDirectory);

        auto renderFileName = std::to_string(juce::Time::currentTimeMillis());

        auto renderFile = userAppDataDirectory.getChildFile(applicationName)
                              .getChildFile("renders")
                              .getNonexistentChildFile(renderFileName, ".wav");

        auto timeRange = tracktion::TimeRange(
            tracktion::TimePosition::fromSeconds(0.0), edit.getLength());
        juce::BigInteger tracksToDo{0};
        for (auto i = 0; i < tracktion::getAllTracks(edit).size(); i++)
            tracksToDo.setBit(i);

        tracktion::Renderer::renderToFile("Render", renderFile, edit, timeRange,
                                          tracksToDo, true, {}, true);
        juce::Logger::writeToLog("Render complete!");
        messageBox.setMessage("Render Complete!");
        // must call resized so message box width is updated to fit text
        resized();
        messageBox.setVisible(true);
        startTimer(1000);
    }
}

void EditTabBarView::mixerButtonReleased() {
    if (isShowing()) {
        juce::StringArray tabNames = getTabNames();
        int index = tabNames.indexOf(mixerTabName);
        if (index != getCurrentTabIndex()) {
            setCurrentTabIndex(index);
            midiCommandManager.setFocusedComponent(
                getCurrentContentComponent());
        }
    }
}

void EditTabBarView::settingsButtonReleased() {
    if (isShowing()) {
        juce::StringArray tabNames = getTabNames();
        int index = tabNames.indexOf(settingsTabName);
        if (index != getCurrentTabIndex()) {
            setCurrentTabIndex(index);
            if (auto navigationController =
                    dynamic_cast<app_navigation::StackNavigationController *>(
                        getCurrentContentComponent())) {
                midiCommandManager.setFocusedComponent(
                    navigationController->getTopComponent());
            }
        } else {
            if (auto navigationController =
                    dynamic_cast<app_navigation::StackNavigationController *>(
                        getCurrentContentComponent())) {
                navigationController->popToRoot();
                midiCommandManager.setFocusedComponent(
                    navigationController->getTopComponent());
            }
        }
    }
}

void EditTabBarView::pluginsButtonReleased() {
    if (isShowing()) {
        juce::StringArray tabNames = getTabNames();
        int index = tabNames.indexOf(pluginsTabName);
        if (index != getCurrentTabIndex()) {
            setCurrentTabIndex(index);
            if (auto navigationController =
                    dynamic_cast<app_navigation::StackNavigationController *>(
                        getCurrentContentComponent())) {
                if (auto pluginView = dynamic_cast<PluginView *>(
                        navigationController->getTopComponent())) {
                    // Four osc has a tab bar component, have to treat it
                    // special
                    if (auto fourOscView = dynamic_cast<FourOscView *>(
                            pluginView->getPlugin()
                                ->windowState->pluginWindow.get()))
                        midiCommandManager.setFocusedComponent(
                            fourOscView->getCurrentContentComponent());
                    else
                        midiCommandManager.setFocusedComponent(
                            pluginView->getPlugin()
                                ->windowState->pluginWindow.get());
                } else {
                    midiCommandManager.setFocusedComponent(
                        navigationController->getTopComponent());
                }
            }

        } else {
            if (auto navigationController =
                    dynamic_cast<app_navigation::StackNavigationController *>(
                        getCurrentContentComponent())) {
                navigationController->popToRoot();
                midiCommandManager.setFocusedComponent(
                    navigationController->getTopComponent());
            }
        }
    }
}

void EditTabBarView::modifiersButtonReleased() {
    if (isShowing()) {
        juce::StringArray tabNames = getTabNames();
        int index = tabNames.indexOf(modifiersTabName);
        if (index != getCurrentTabIndex()) {
            setCurrentTabIndex(index);
            if (auto navigationController =
                    dynamic_cast<app_navigation::StackNavigationController *>(
                        getCurrentContentComponent()))
                midiCommandManager.setFocusedComponent(
                    navigationController->getTopComponent());

        } else {
            if (auto navigationController =
                    dynamic_cast<app_navigation::StackNavigationController *>(
                        getCurrentContentComponent())) {
                navigationController->popToRoot();
                midiCommandManager.setFocusedComponent(
                    navigationController->getTopComponent());
            }
        }
    }
}

void EditTabBarView::sequencersButtonReleased() {
    if (isShowing()) {
        juce::StringArray tabNames = getTabNames();
        int index = tabNames.indexOf(sequencersTabName);
        if (index != getCurrentTabIndex()) {
            setCurrentTabIndex(index);
            if (auto navigationController =
                    dynamic_cast<app_navigation::StackNavigationController *>(
                        getCurrentContentComponent()))
                midiCommandManager.setFocusedComponent(
                    navigationController->getTopComponent());

        } else {
            if (auto navigationController =
                    dynamic_cast<app_navigation::StackNavigationController *>(
                        getCurrentContentComponent())) {
                navigationController->popToRoot();
                midiCommandManager.setFocusedComponent(
                    navigationController->getTopComponent());
            }
        }
    }
}

void EditTabBarView::selectedIndexChanged(int newIndex) {
    resetTrackRelatedTabs();
}

void EditTabBarView::resetModifiersTab() {
    juce::StringArray tabNames = getTabNames();
    int tracksIndex = tabNames.indexOf(tracksTabName);
    if (auto tracksView =
            dynamic_cast<TracksView *>(getTabContentComponent(tracksIndex))) {
        if (auto track = dynamic_cast<tracktion::AudioTrack *>(
                tracksView->getViewModel().listViewModel.getSelectedItem())) {
            tabNames = getTabNames();
            int modifiersIndex = tabNames.indexOf(modifiersTabName);
            removeTab(modifiersIndex);

            addTab(modifiersTabName, juce::Colours::transparentBlack,
                   new app_navigation::StackNavigationController(
                       new TrackModifiersListView(track, midiCommandManager)),
                   true);
        }
    }
}

void EditTabBarView::octaveChanged(int newOctave) {
    viewModel.setCurrentOctave(newOctave);
    octaveDisplayComponent.setOctave(newOctave);
    octaveDisplayComponent.setVisible(true);
    startTimer(1000);
}

void EditTabBarView::timerCallback() {
    octaveDisplayComponent.setVisible(false);
    messageBox.setVisible(false);
}

void EditTabBarView::currentTabChanged(int newCurrentTabIndex,
                                       const juce::String &newCurrentTabName) {
    // A bunch of stuff happens in the sequencer view constructor
    // that needs to happen everytime it comes on screen
    // easiest way is just to remove and add the sequencers tab anytime a tab is
    // changed A better way would be to do this only when the SEQUENCER tab is
    // navigated away from, but this works for now The best way would be to
    // detect when the sequencer tab is shown and run some init method or
    // something
    if (newCurrentTabName != sequencersTabName) {
        juce::StringArray tabNames = getTabNames();
        int tracksIndex = tabNames.indexOf(tracksTabName);
        if (auto tracksView = dynamic_cast<TracksView *>(
                getTabContentComponent(tracksIndex))) {
            if (auto track = dynamic_cast<tracktion::AudioTrack *>(
                    tracksView->getViewModel()
                        .listViewModel.getSelectedItem())) {
                tabNames = getTabNames();
                int sequencersIndex = tabNames.indexOf(sequencersTabName);
                removeTab(sequencersIndex);

                addTab(sequencersTabName, juce::Colours::transparentBlack,
                       new app_navigation::StackNavigationController(
                           new AvailableSequencersListView(track,
                                                           midiCommandManager)),
                       true);
            }
        }
    }
}

void EditTabBarView::trackDeleted() { resetTrackRelatedTabs(); }

void EditTabBarView::resetTrackRelatedTabs() {
    juce::StringArray tabNames = getTabNames();
    int tracksIndex = tabNames.indexOf(tracksTabName);
    if (auto tracksView =
            dynamic_cast<TracksView *>(getTabContentComponent(tracksIndex))) {
        if (auto track = dynamic_cast<tracktion::AudioTrack *>(
                tracksView->getViewModel().listViewModel.getSelectedItem())) {
            tabNames = getTabNames();
            int sequencersIndex = tabNames.indexOf(sequencersTabName);
            removeTab(sequencersIndex);

            tabNames = getTabNames();
            int modifiersIndex = tabNames.indexOf(modifiersTabName);
            removeTab(modifiersIndex);

            tabNames = getTabNames();
            int pluginsIndex = tabNames.indexOf(pluginsTabName);
            removeTab(pluginsIndex);

            addTab(pluginsTabName, juce::Colours::transparentBlack,
                   new app_navigation::StackNavigationController(
                       new TrackPluginsListView(track, midiCommandManager)),
                   true);
            addTab(modifiersTabName, juce::Colours::transparentBlack,
                   new app_navigation::StackNavigationController(
                       new TrackModifiersListView(track, midiCommandManager)),
                   true);
            addTab(
                sequencersTabName, juce::Colours::transparentBlack,
                new app_navigation::StackNavigationController(
                    new AvailableSequencersListView(track, midiCommandManager)),
                true);
        }
    }
}
