#include "TracksView.h"
#include "AppLookAndFeel.h"
#include "MixerView.h"
#include <app_navigation/app_navigation.h>

TracksView::TracksView(tracktion::Edit &e,
                       app_services::MidiCommandManager &mcm)
    : edit(e), midiCommandManager(mcm), camera(7), viewModel(e, camera),
      listModel(std::make_unique<TracksListBoxModel>(viewModel.listViewModel,
                                                     camera)),
      singleTrackView(std::make_unique<TrackView>(
          dynamic_cast<tracktion::AudioTrack *>(
              viewModel.listViewModel.getSelectedItem()),
          camera)) {
    multiTrackListBox.setModel(listModel.get());
    multiTrackListBox.getViewport()->setScrollBarsShown(false, false);
    multiTrackListBox.setColour(juce::ListBox::backgroundColourId,
                                juce::Colour(0x00282828));
    multiTrackListBox.setAlwaysOnTop(true);
    addAndMakeVisible(multiTrackListBox);

    addAndMakeVisible(singleTrackView.get());

    addAndMakeVisible(informationPanel);

    playheadComponent.setAlwaysOnTop(true);
    addAndMakeVisible(playheadComponent);

    loopMarkerComponent.setAlwaysOnTop(true);
    addAndMakeVisible(loopMarkerComponent);
    loopMarkerComponent.setVisible(false);

    midiCommandManager.addListener(this);
    // since this is the initial view we will manually set it to be the focused
    // component
    midiCommandManager.setFocusedComponent(this);

    viewModel.addListener(this);
    viewModel.listViewModel.addListener(this);
    viewModel.listViewModel.itemListState.addListener(this);

    startTimerHz(60);
}

TracksView::~TracksView() {
    midiCommandManager.removeListener(this);
    viewModel.removeListener(this);
    viewModel.listViewModel.removeListener(this);
    viewModel.listViewModel.itemListState.removeListener(this);
    stopTimer();
}

void TracksView::paint(juce::Graphics &g) {
    g.fillAll(
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colours::white);
}

void TracksView::resized() {
    informationPanel.setBounds(0, 0, getWidth(), getHeight() / 4);

    singleTrackView->setBounds(0, informationPanel.getHeight(), getWidth(),
                               getHeight() - informationPanel.getHeight());

    multiTrackListBox.setBounds(0, informationPanel.getHeight(), getWidth(),
                                getHeight() - informationPanel.getHeight());
    multiTrackListBox.setRowHeight(getHeight() / 6);
    multiTrackListBox.scrollToEnsureRowIsOnscreen(
        viewModel.listViewModel.itemListState.getSelectedItemIndex());
}

void TracksView::encoder1Increased() {
    if (isShowing())
        if (midiCommandManager.getFocusedComponent() == this)
            viewModel.listViewModel.itemListState.setSelectedItemIndex(
                viewModel.listViewModel.itemListState.getSelectedItemIndex() +
                1);
}

void TracksView::encoder1Decreased() {
    if (isShowing())
        if (midiCommandManager.getFocusedComponent() == this)
            viewModel.listViewModel.itemListState.setSelectedItemIndex(
                viewModel.listViewModel.itemListState.getSelectedItemIndex() -
                1);
}

void TracksView::encoder1ButtonReleased() {
    if (isShowing()) {
        if (midiCommandManager.getFocusedComponent() == this) {
            if (viewModel.getTracksViewType() ==
                app_view_models::TracksListViewModel::TracksViewType::
                    MULTI_TRACK)
                viewModel.setTracksViewType(
                    app_view_models::TracksListViewModel::TracksViewType::
                        SINGLE_TRACK);
            else
                viewModel.setTracksViewType(
                    app_view_models::TracksListViewModel::TracksViewType::
                        MULTI_TRACK);

            juce::Timer::callAfterDelay(1,
                                        [this]() { sendLookAndFeelChange(); });
        }
    }
}

void TracksView::encoder2Increased() {
    if (isShowing()) {
        if (midiCommandManager.getFocusedComponent() == this) {
            if (midiCommandManager.isControlDown)
                viewModel.nudgeLoopInForwardToNearestBeat();
            else
                viewModel.nudgeLoopOutForwardToNearestBeat();
        }
    }
}
void TracksView::encoder2Decreased() {
    if (isShowing()) {
        if (midiCommandManager.getFocusedComponent() == this) {
            if (midiCommandManager.isControlDown)
                viewModel.nudgeLoopInBackwardToNearestBeat();
            else
                viewModel.nudgeLoopOutBackwardToNearestBeat();
        }
    }
}

void TracksView::encoder3Increased() {
    if (isShowing()) {
        if (midiCommandManager.getFocusedComponent() == this) {
            if (midiCommandManager.isControlDown)
                viewModel.nudgeTransportForwardToNearestBeat();
            else
                viewModel.nudgeTransportForward();
        }
    }
}

void TracksView::encoder3Decreased() {
    if (isShowing()) {
        if (midiCommandManager.getFocusedComponent() == this) {
            if (midiCommandManager.isControlDown)
                viewModel.nudgeTransportBackwardToNearestBeat();
            else
                viewModel.nudgeTransportBackward();
        }
    }
}

void TracksView::encoder3ButtonReleased() { viewModel.toggleSolo(); }

void TracksView::encoder4Increased() {
    int colourIndex =
        appLookAndFeel.colours.indexOf(viewModel.getSelectedTrackColour());
    if (colourIndex == appLookAndFeel.colours.size() - 1)
        colourIndex = 0;
    else
        colourIndex++;

    viewModel.setSelectedTrackColour(appLookAndFeel.colours[colourIndex]);
}

void TracksView::encoder4Decreased() {
    int colourIndex =
        appLookAndFeel.colours.indexOf(viewModel.getSelectedTrackColour());
    if (colourIndex == 0)
        colourIndex = appLookAndFeel.colours.size() - 1;
    else
        colourIndex--;

    viewModel.setSelectedTrackColour(appLookAndFeel.colours[colourIndex]);
}

void TracksView::encoder4ButtonReleased() { viewModel.toggleMute(); }

void TracksView::cutButtonReleased() {
    if (isShowing())
        if (midiCommandManager.getFocusedComponent() == this)
            viewModel.cutSelectedTracksClipAtPlayHead();
}

void TracksView::pasteButtonReleased() {
    if (isShowing())
        if (midiCommandManager.getFocusedComponent() == this)
            viewModel.pasteClipboardContentToTrackAtPlayhead();
}

void TracksView::sliceButtonReleased() {
    if (isShowing())
        if (midiCommandManager.getFocusedComponent() == this) {
            if (midiCommandManager.isControlDown)
                viewModel.mergeSelectedTracksClipsAtPlayhead();
            else
                viewModel.splitSelectedTracksClipAtPlayHead();
        }
}

void TracksView::plusButtonReleased() {
    if (isShowing())
        if (midiCommandManager.getFocusedComponent() == this) {
            viewModel.addTrack();
            shouldUpdateTrackColour = true;
        }
}

void TracksView::minusButtonReleased() {
    if (isShowing())
        if (midiCommandManager.getFocusedComponent() == this)
            viewModel.deleteSelectedTrack();
}

void TracksView::loopingChanged(bool looping) {
    informationPanel.setIsLooping(looping);
    loopMarkerComponent.setVisible(looping);
    resized();
}
void TracksView::loopInButtonReleased() {
    if (isShowing())
        if (midiCommandManager.getFocusedComponent() == this)
            viewModel.setLoopIn();
}

void TracksView::loopOutButtonReleased() {
    if (isShowing())
        if (midiCommandManager.getFocusedComponent() == this)
            viewModel.setLoopOut();
}

void TracksView::loopButtonReleased() {
    if (isShowing())
        if (midiCommandManager.getFocusedComponent() == this)
            viewModel.toggleLooping();
}

void TracksView::recordButtonReleased() {
    if (isShowing())
        if (midiCommandManager.getFocusedComponent() == this)
            viewModel.startRecording();
}

void TracksView::playButtonReleased() {
    if (isShowing())
        if (midiCommandManager.getFocusedComponent() == this)
            viewModel.startPlaying();
}

void TracksView::stopButtonReleased() {
    if (isShowing())
        if (midiCommandManager.getFocusedComponent() == this)
            viewModel.stopRecordingOrPlaying();
}

void TracksView::selectedIndexChanged(int newIndex) {
    multiTrackListBox.updateContent();
    multiTrackListBox.selectRow(
        viewModel.listViewModel.itemListState.getSelectedItemIndex());

    if (auto track = dynamic_cast<tracktion::AudioTrack *>(
            viewModel.listViewModel.getSelectedItem())) {
        singleTrackView.reset();
        singleTrackView = std::make_unique<TrackView>(*track, camera);
        singleTrackView->setSelected(true);
        singleTrackView->setBounds(0, informationPanel.getHeight(), getWidth(),
                                   getHeight() - informationPanel.getHeight());
        singleTrackView->setAlwaysOnTop(true);
        addChildComponent(singleTrackView.get());
        playheadComponent.setAlwaysOnTop(true);
        addAndMakeVisible(playheadComponent);
        playheadComponent.toFront(false);
        loopMarkerComponent.toFront(false);
    }

    if (viewModel.getTracksViewType() ==
        app_view_models::TracksListViewModel::TracksViewType::SINGLE_TRACK) {
        singleTrackView->setVisible(true);
        multiTrackListBox.setVisible(false);
    } else {
        singleTrackView->setVisible(false);
        multiTrackListBox.setVisible(true);
    }

    // When deleting tracks in quick succession sometimes the selectedItem is
    // null
    if (viewModel.listViewModel.getSelectedItem() != nullptr) {
        informationPanel.setTrackNumber(
            viewModel.listViewModel.getSelectedItem()
                ->getName()
                .trimCharactersAtStart("Track "));
        informationPanel.setIsSoloed(viewModel.getSelectedTrackSoloState());
        informationPanel.setIsMuted(viewModel.getSelectedTrackMuteState());
    }

    sendLookAndFeelChange();
    repaint();
    resized();
}

void TracksView::isRecordingChanged(bool isRecording) {
    informationPanel.setIsRecording(isRecording);
}

void TracksView::isPlayingChanged(bool isPlaying) {
    informationPanel.setIsPlaying(isPlaying);
}

void TracksView::itemsChanged() {
    multiTrackListBox.updateContent();
    multiTrackListBox.scrollToEnsureRowIsOnscreen(
        viewModel.listViewModel.itemListState.getSelectedItemIndex());

    if (auto track = dynamic_cast<tracktion::AudioTrack *>(
            viewModel.listViewModel.getSelectedItem())) {
        if (shouldUpdateTrackColour) {
            track->setColour(appLookAndFeel.getRandomColour());
            shouldUpdateTrackColour = false;
        }

        singleTrackView.reset();
        singleTrackView = std::make_unique<TrackView>(*track, camera);
        singleTrackView->setSelected(true);
        singleTrackView->setBounds(0, informationPanel.getHeight(), getWidth(),
                                   getHeight() - informationPanel.getHeight());
        singleTrackView->setAlwaysOnTop(true);
        addChildComponent(singleTrackView.get());
        if (viewModel.getTracksViewType() ==
            app_view_models::TracksListViewModel::TracksViewType::SINGLE_TRACK)
            singleTrackView->setVisible(true);

        playheadComponent.setAlwaysOnTop(true);
        addAndMakeVisible(playheadComponent);
        playheadComponent.toFront(false);
        loopMarkerComponent.toFront(false);
    }

    sendLookAndFeelChange();
    resized();
    repaint();
}

void TracksView::tracksViewTypeChanged(
    app_view_models::TracksListViewModel::TracksViewType type) {
    switch (type) {
    case app_view_models::TracksListViewModel::TracksViewType::SINGLE_TRACK:

        singleTrackView->setVisible(true);
        multiTrackListBox.setVisible(false);
        break;

    case app_view_models::TracksListViewModel::TracksViewType::MULTI_TRACK:

        multiTrackListBox.setVisible(true);
        singleTrackView->setVisible(false);
        break;

    default:

        multiTrackListBox.setVisible(true);
        singleTrackView->setVisible(false);
        break;
    }

    sendLookAndFeelChange();
    resized();
    repaint();
}

void TracksView::soloStateChanged(bool solo) {
    // really need to refactor the info component to handle all this stuff but
    // its whatever
    informationPanel.setIsSoloed(solo);
}
void TracksView::muteStateChanged(bool mute) {
    informationPanel.setIsMuted(mute);
}

void TracksView::buildBeats() {
    juce::Colour beatColour = appLookAndFeel.colour3.darker(.5f);
    beats.clear();

    double pxPerSec = getWidth() / camera.getScope();
    double secondsPerBeat =
        (1.0 / edit.tempoSequence.getBeatsPerSecondAt(
                   tracktion::TimePosition::fromSeconds(0.0)));
    // give a few extra beats per screen
    int beatsPerScreen =
        static_cast<int>((camera.getScope() / secondsPerBeat) + 2);
    double pxPerBeat = secondsPerBeat * pxPerSec;
    auto leftEdgeBeat = edit.tempoSequence
                            .toBeats(tracktion::TimePosition::fromSeconds(
                                camera.getCenter() - (camera.getScope() / 2.0)))
                            .inBeats();
    double beatOffset = ceil(leftEdgeBeat) - leftEdgeBeat;
    double timeOffset = secondsPerBeat * beatOffset;

    double pxOffset = timeOffset * pxPerSec;

    int leftEdgeBeatNumber = static_cast<int>(ceil(leftEdgeBeat));

    for (int i = 0; i < beatsPerScreen; i++) {
        double beatX = (i * pxPerBeat) + pxOffset;
        int beatNumber = leftEdgeBeatNumber + i;

        beats.add(new juce::DrawableRectangle());
        beats.getLast()->setFill(juce::FillType(beatColour));
        beats.getLast()->setStrokeFill(juce::FillType(beatColour));
        juce::Point<float> topLeft(beatX - .5, informationPanel.getHeight());
        juce::Point<float> topRight(beatX + .5, informationPanel.getHeight());
        juce::Point<float> bottomLeft(beatX - .5, getHeight());
        juce::Parallelogram<float> bounds(topLeft, topRight, bottomLeft);
        beats.getLast()->setRectangle(bounds);

        if (beatNumber % 4 == 0) {
            juce::Point<float> topLeft(beatX - 1.5,
                                       informationPanel.getHeight());
            juce::Point<float> topRight(beatX + 1.5,
                                        informationPanel.getHeight());
            juce::Point<float> bottomLeft(beatX - 1.5, getHeight());
            juce::Parallelogram<float> bounds(topLeft, topRight, bottomLeft);
            beats.getLast()->setRectangle(bounds);
        }

        addAndMakeVisible(beats.getLast());
    }
}

void TracksView::timerCallback() {
    informationPanel.setTimecode(edit.getTimecodeFormat().getString(
        edit.tempoSequence, edit.getTransport().getPosition(), false));
    playheadComponent.setBounds(
        camera.timeToX(edit.getTransport().getCurrentPosition(), getWidth()),
        informationPanel.getHeight(), 2,
        getHeight() - informationPanel.getHeight());

    double loop1X =
        camera.timeToX(edit.getTransport().loopPoint1->inSeconds(), getWidth());
    double loop2X =
        camera.timeToX(edit.getTransport().loopPoint2->inSeconds(), getWidth());
    double loopEndpointRadius = 12;
    loopMarkerComponent.setBounds(
        loop1X - loopEndpointRadius,
        informationPanel.getHeight() - loopEndpointRadius,
        loop2X - loop1X + 2 * loopEndpointRadius, 2 * loopEndpointRadius);

    buildBeats();
    repaint();
}

void TracksView::undoButtonReleased() {
    if (isShowing())
        if (midiCommandManager.getFocusedComponent() == this)
            viewModel.undo();
}

// CJM
void TracksView::noteOnPressed(int noteNumber) {
    juce::Logger::writeToLog("noteOnPressed  noteNumber" +
                             std::to_string(noteNumber));
    if (midiCommandManager.isPlusDown) {
        juce::Logger::writeToLog("Ha entrado en el mute");

        int trackIndex = -1;

        switch (noteNumber) {
        case 53:
            trackIndex = 1;
            break;
        case 54:
            trackIndex = 2;
            break;
        case 55:
            trackIndex = 3;
            break;
        case 56:
            trackIndex = 4;
            break;
        default:
            break;
        }
        if (trackIndex != -1) {
            // Verifica si el índice es válido
            if (trackIndex < viewModel.listViewModel.itemListState.listSize) {
                viewModel.listViewModel.itemListState.setSelectedItemIndex(
                    trackIndex);
                informationPanel.setIsMuted(
                    viewModel.getSelectedTrackMuteState());
            }
        }
    }

}