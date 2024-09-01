#include "TracksListViewModel.h"

namespace app_view_models {

TracksListViewModel::TracksListViewModel(tracktion::Edit &e,
                                         app_services::TimelineCamera &cam)
    : edit(e),

      camera(cam), adapter(std::make_unique<TracksListAdapter>(edit)),
      state(edit.state.getOrCreateChildWithName(IDs::TRACKS_LIST_VIEW_STATE,
                                                nullptr)),
      listViewModel(edit.state, state, tracktion::IDs::TRACK, adapter.get()) {
    initialiseInputs();
    listViewModel.itemListState.addListener(this);
    edit.state.addListener(this);
    edit.getTransport().addChangeListener(this);
    edit.getTransport().addListener(this);
    edit.getTransport().state.addListener(this);

    edit.getUndoManager().clearUndoHistory();

    tracksViewType.referTo(state, IDs::tracksListViewType, nullptr,
                           static_cast<int>(TracksViewType::MULTI_TRACK));

    edit.getTransport().setCurrentPosition(0.0);
}

TracksListViewModel::~TracksListViewModel() {
    listViewModel.removeListener(this);
    edit.state.removeListener(this);
    edit.getTransport().removeChangeListener(this);
    edit.getTransport().removeListener(this);
}

void TracksListViewModel::initialiseInputs() {
    // set initial midi devices
    auto &deviceManager = edit.engine.getDeviceManager();
    for (int i = 0; i < deviceManager.getNumMidiInDevices(); i++) {
        if (auto midiInputDevice = deviceManager.getMidiInDevice(i)) {
            midiInputDevice->setEndToEndEnabled(true);
            midiInputDevice->setEnabled(true);
        }
    }

    edit.getTransport().ensureContextAllocated();

    for (auto instance : edit.getAllInputDevices()) {
        if (instance->getInputDevice().getDeviceType() ==
            tracktion::InputDevice::physicalMidiDevice) {
            if (auto selectedTrack = dynamic_cast<tracktion::AudioTrack *>(
                    listViewModel.getSelectedItem())) {
                instance->setTargetTrack(*selectedTrack, 0, true);
                instance->setRecordingEnabled(*selectedTrack, true);
            }
        }
    }

    edit.restartPlayback();
}

void TracksListViewModel::addTrack() {
    edit.ensureNumberOfAudioTracks(listViewModel.getAdapter()->size() + 1);

    // since we are manually settings the selected index, we need to update the
    // size of the list as well.
    listViewModel.itemListState.listSize =
        listViewModel.getAdapter()->size() + 1;
    listViewModel.itemListState.setSelectedItemIndex(
        listViewModel.getAdapter()->size() - 1);
}

void TracksListViewModel::deleteSelectedTrack() {
    if (auto track = dynamic_cast<tracktion::AudioTrack *>(
            listViewModel.getSelectedItem())) {
        // dont allow user to delete only remaining track
        if (listViewModel.getAdapter()->size() > 1) {
            edit.deleteTrack(track);

            // after deleting the track, we need to initialize the selected
            // tracks input
            initialiseInputs();
        }
    }
}

void TracksListViewModel::splitSelectedTracksClipAtPlayHead() {
    if (auto track = dynamic_cast<tracktion::AudioTrack *>(
            listViewModel.getSelectedItem())) {
        if (auto trackItem = track->getNextTrackItemAt(
                track->edit.getTransport().getPosition())) {
            if (track->edit.getTransport().getPosition() >=
                    trackItem->getPosition().getStart() &&
                track->edit.getTransport().getPosition() <=
                    trackItem->getPosition().getEnd()) {
                if (auto clip = dynamic_cast<tracktion::Clip *>(trackItem)) {
                    if (auto clipTrack = clip->getClipTrack()) {
                        clipTrack->splitClip(*clip,
                                             edit.getTransport().getPosition());
                    }
                }
            }
        }
    }
}

void TracksListViewModel::cutSelectedTracksClipAtPlayHead() {
    if (auto track = dynamic_cast<tracktion::AudioTrack *>(
            listViewModel.getSelectedItem())) {
        if (auto trackItem = track->getNextTrackItemAt(
                track->edit.getTransport().getPosition())) {
            if (track->edit.getTransport().getPosition() >=
                    trackItem->getPosition().getStart() &&
                track->edit.getTransport().getPosition() <=
                    trackItem->getPosition().getEnd()) {
                if (auto clip = dynamic_cast<tracktion::Clip *>(trackItem)) {
                    tracktion::Clipboard::getInstance()->clear();
                    auto clipContent =
                        std::make_unique<tracktion::Clipboard::Clips>();
                    clipContent->addClip(0, clip->state);
                    tracktion::Clipboard::getInstance()->setContent(
                        std::move(clipContent));
                    clip->removeFromParentTrack();
                }
            }
        }
    }
}

void TracksListViewModel::mergeSelectedTracksClipsAtPlayhead() {
    if (auto track = dynamic_cast<tracktion::AudioTrack *>(
            listViewModel.getSelectedItem())) {
        if (auto clip1 = dynamic_cast<tracktion::MidiClip *>(
                track->getNextTrackItemAt(tracktion::TimePosition::fromSeconds(
                    track->edit.getTransport().getCurrentPosition() - .1)))) {
            if (auto clip2 = dynamic_cast<tracktion::MidiClip *>(
                    track->getNextTrackItemAt(
                        tracktion::TimePosition::fromSeconds(
                            track->edit.getTransport().getCurrentPosition() +
                            .1)))) {
                if (clip1->itemID != clip2->itemID) {
                    juce::Array<tracktion::MidiClip *> clips;
                    clips.add(clip1);
                    clips.add(clip2);
                    tracktion::mergeMidiClips(clips);
                }
            }
        }
    }
}

void TracksListViewModel::pasteClipboardContentToTrackAtPlayhead() {
    if (auto track = dynamic_cast<tracktion::AudioTrack *>(
            listViewModel.getSelectedItem())) {
        auto clipboard = tracktion::Clipboard::getInstance();
        if (!clipboard->isEmpty()) {
            if (clipboard->hasContentWithType<tracktion::Clipboard::Clips>()) {
                auto clipContent =
                    clipboard
                        ->getContentWithType<tracktion::Clipboard::Clips>();
                tracktion::EditInsertPoint insertPoint(edit);
                insertPoint.setNextInsertPoint(
                    edit.getTransport().getPosition(), track);

                tracktion::Clipboard::ContentType::EditPastingOptions options(
                    edit, insertPoint);

                double start = 0;
                for (auto &i : clipContent->clips) {
                    auto end = i.hasBeatTimes
                                   ? edit.tempoSequence.toTime(i.startBeats)
                                         .inSeconds()
                                   : (static_cast<double>(i.state.getProperty(
                                         tracktion::IDs::start)));

                    start = std::max(start, end);
                }

                options.startTime = tracktion::TimePosition::fromSeconds(
                    edit.getTransport().getCurrentPosition() - start);
                clipContent->pasteIntoEdit(options);
                edit.getTransport().setCurrentPosition(
                    edit.getTransport().getCurrentPosition() +
                    static_cast<double>(clipContent->clips[0].state.getProperty(
                        tracktion::IDs::length)));
            }
        }
    }
}

TracksListViewModel::TracksViewType TracksListViewModel::getTracksViewType() {
    return static_cast<TracksViewType>(tracksViewType.get());
}

void TracksListViewModel::setTracksViewType(TracksViewType type) {
    tracksViewType.setValue(static_cast<int>(type), nullptr);
}
void TracksListViewModel::startRecording() {
    // ensure a selected track exists

    edit.clickTrackEnabled = true;

    if (auto selectedTrack = dynamic_cast<tracktion::AudioTrack *>(
            listViewModel.getSelectedItem())) {
        // only start recording if we currently arent recording
        // and the selected track is armed
        if (EngineHelpers::isTrackArmed(*selectedTrack)) {
            auto &transport = edit.getTransport();
            if (!transport.isRecording() && !transport.isPlaying()) {
                transport.record(false);
            }
        }
    }
}

void TracksListViewModel::startPlaying() {
    edit.clickTrackEnabled = false;

    auto &transport = edit.getTransport();

    if (!transport.isPlaying()) {
        transport.play(false);
    }
}

void TracksListViewModel::stopRecordingOrPlaying() {
    auto &transport = edit.getTransport();
    if (transport.isPlaying() || transport.isRecording()) {
        transport.stop(false, false);

        tracktion::EditFileOperations fileOperations(edit);
        fileOperations.save(true, true, false);

    } else {
        // if we try to stop while currently not playing
        // return transport to beginning
        transport.setCurrentPosition(0.0);
    }
}

void TracksListViewModel::nudgeTransportForward() {
    auto &transport = edit.getTransport();
    if (!transport.isPlaying() && !transport.isRecording()) {
        // use the camera's nudge amount to nudge both the transport and camera
        transport.setCurrentPosition(transport.getCurrentPosition() +
                                     camera.getNudgeAmount());
    }
}

void TracksListViewModel::nudgeTransportBackward() {
    auto &transport = edit.getTransport();
    if (!transport.isPlaying() && !transport.isRecording()) {
        if (transport.getCurrentPosition() > camera.getNudgeAmount() + .01)
            transport.setCurrentPosition(transport.getCurrentPosition() -
                                         camera.getNudgeAmount());
        else
            transport.setCurrentPosition(0);
    }
}

void TracksListViewModel::nudgeTransportForwardToNearestBeat() {
    double nearestBeatTime =
        tracktion::TimecodeSnapType::get1BeatSnapType()
            .roundTimeUp(edit.getTransport().getPosition(), edit.tempoSequence)
            .inSeconds();

    // Check if we are on a beat already
    if (nearestBeatTime == edit.getTransport().getCurrentPosition()) {
        double secondsPerBeat =
            1.0 / edit.tempoSequence.getBeatsPerSecondAt(
                      tracktion::TimePosition::fromSeconds(0.0));
        edit.getTransport().setCurrentPosition(
            edit.getTransport().getCurrentPosition() + secondsPerBeat);

    } else {
        edit.getTransport().setCurrentPosition(nearestBeatTime);
    }
}

void TracksListViewModel::nudgeTransportBackwardToNearestBeat() {
    double nearestBeatTime =
        tracktion::TimecodeSnapType::get1BeatSnapType()
            .roundTimeDown(edit.getTransport().getPosition(),
                           edit.tempoSequence)
            .inSeconds();

    // Check if we are on a beat already
    if (nearestBeatTime == edit.getTransport().getCurrentPosition()) {
        double secondsPerBeat =
            1.0 / edit.tempoSequence.getBeatsPerSecondAt(
                      tracktion::TimePosition::fromSeconds(0.0));
        edit.getTransport().setCurrentPosition(
            edit.getTransport().getCurrentPosition() - secondsPerBeat);

    } else {
        edit.getTransport().setCurrentPosition(nearestBeatTime);
    }
}

void TracksListViewModel::setLoopIn() {
    if (!edit.getTransport().looping)
        edit.getTransport().looping.setValue(true, nullptr);

    double currentOut = edit.getTransport().loopPoint2->inSeconds();
    edit.getTransport().setLoopIn(edit.getTransport().getPosition());

    if (edit.getTransport().loopPoint1->inSeconds() >= currentOut)
        edit.getTransport().setLoopOut(edit.getTransport().loopPoint1.get());
    else
        edit.getTransport().setLoopOut(
            tracktion::TimePosition::fromSeconds(currentOut));
}

void TracksListViewModel::setLoopOut() {
    if (!edit.getTransport().looping)
        edit.getTransport().looping.setValue(true, nullptr);

    edit.getTransport().setLoopOut(edit.getTransport().getPosition());
}

void TracksListViewModel::toggleLooping() {
    edit.getTransport().looping.setValue(!edit.getTransport().looping.get(),
                                         nullptr);
}

void TracksListViewModel::nudgeLoopInForward() {
    if (!edit.getTransport().looping)
        edit.getTransport().looping.setValue(true, nullptr);

    edit.getTransport().setPosition(edit.getTransport().loopPoint1);
    nudgeTransportForward();
    setLoopIn();
}
void TracksListViewModel::nudgeLoopInBackward() {
    if (!edit.getTransport().looping)
        edit.getTransport().looping.setValue(true, nullptr);

    edit.getTransport().setPosition(edit.getTransport().loopPoint1);
    nudgeTransportBackward();
    setLoopIn();
}

void TracksListViewModel::nudgeLoopOutForward() {
    if (!edit.getTransport().looping)
        edit.getTransport().looping.setValue(true, nullptr);

    edit.getTransport().setPosition(edit.getTransport().loopPoint2);
    nudgeTransportForward();
    setLoopOut();
}
void TracksListViewModel::nudgeLoopOutBackward() {
    if (!edit.getTransport().looping)
        edit.getTransport().looping.setValue(true, nullptr);

    edit.getTransport().setPosition(edit.getTransport().loopPoint2);
    nudgeTransportBackward();
    setLoopOut();
}

void TracksListViewModel::nudgeLoopInForwardToNearestBeat() {
    if (!edit.getTransport().looping)
        edit.getTransport().looping.setValue(true, nullptr);
    edit.getTransport().setPosition(edit.getTransport().loopPoint1);

    double nearestBeatTime =
        tracktion::TimecodeSnapType::get1BeatSnapType()
            .roundTimeUp(edit.getTransport().getPosition(), edit.tempoSequence)
            .inSeconds();

    // Check if we are on a beat already
    if (nearestBeatTime == edit.getTransport().getCurrentPosition()) {
        double secondsPerBeat =
            1.0 / edit.tempoSequence.getBeatsPerSecondAt(
                      tracktion::TimePosition::fromSeconds(0.0));
        edit.getTransport().setCurrentPosition(
            edit.getTransport().getCurrentPosition() + secondsPerBeat);

    } else {
        edit.getTransport().setCurrentPosition(nearestBeatTime);
    }

    setLoopIn();
}

void TracksListViewModel::nudgeLoopInBackwardToNearestBeat() {
    if (!edit.getTransport().looping)
        edit.getTransport().looping.setValue(true, nullptr);

    edit.getTransport().setPosition(edit.getTransport().loopPoint1);

    double nearestBeatTime =
        tracktion::TimecodeSnapType::get1BeatSnapType()
            .roundTimeDown(edit.getTransport().getPosition(),
                           edit.tempoSequence)
            .inSeconds();

    // Check if we are on a beat already
    if (nearestBeatTime == edit.getTransport().getCurrentPosition()) {
        double secondsPerBeat =
            1.0 / edit.tempoSequence.getBeatsPerSecondAt(
                      tracktion::TimePosition::fromSeconds(0.0));
        edit.getTransport().setCurrentPosition(
            edit.getTransport().getCurrentPosition() - secondsPerBeat);

    } else {
        edit.getTransport().setCurrentPosition(nearestBeatTime);
    }

    setLoopIn();
}

void TracksListViewModel::nudgeLoopOutForwardToNearestBeat() {
    if (!edit.getTransport().looping)
        edit.getTransport().looping.setValue(true, nullptr);
    edit.getTransport().setPosition(edit.getTransport().loopPoint2);

    double nearestBeatTime =
        tracktion::TimecodeSnapType::get1BeatSnapType()
            .roundTimeUp(edit.getTransport().getPosition(), edit.tempoSequence)
            .inSeconds();

    // Check if we are on a beat already
    if (nearestBeatTime == edit.getTransport().getCurrentPosition()) {
        double secondsPerBeat =
            1.0 / edit.tempoSequence.getBeatsPerSecondAt(
                      tracktion::TimePosition::fromSeconds(0.0));
        edit.getTransport().setCurrentPosition(
            edit.getTransport().getCurrentPosition() + secondsPerBeat);

    } else {
        edit.getTransport().setCurrentPosition(nearestBeatTime);
    }

    setLoopOut();
}

void TracksListViewModel::nudgeLoopOutBackwardToNearestBeat() {
    if (!edit.getTransport().looping)
        edit.getTransport().looping.setValue(true, nullptr);

    edit.getTransport().setPosition(edit.getTransport().loopPoint2);

    double nearestBeatTime =
        tracktion::TimecodeSnapType::get1BeatSnapType()
            .roundTimeDown(edit.getTransport().getPosition(),
                           edit.tempoSequence)
            .inSeconds();

    // Check if we are on a beat already
    if (nearestBeatTime == edit.getTransport().getCurrentPosition()) {
        double secondsPerBeat =
            1.0 / edit.tempoSequence.getBeatsPerSecondAt(
                      tracktion::TimePosition::fromSeconds(0.0));
        edit.getTransport().setCurrentPosition(
            edit.getTransport().getCurrentPosition() - secondsPerBeat);

    } else {
        edit.getTransport().setCurrentPosition(nearestBeatTime);
    }

    setLoopOut();
}

bool TracksListViewModel::getSelectedTrackSoloState() {
    if (auto selectedTrack = dynamic_cast<tracktion::AudioTrack *>(
            listViewModel.getSelectedItem()))
        return selectedTrack->isSolo(false);
    else
        return false;
}

bool TracksListViewModel::getSelectedTrackMuteState() {
    if (auto selectedTrack = dynamic_cast<tracktion::AudioTrack *>(
            listViewModel.getSelectedItem()))
        return selectedTrack->isMuted(false);
    else
        return false;
}

void TracksListViewModel::toggleSolo() {
    if (auto selectedTrack = dynamic_cast<tracktion::AudioTrack *>(
            listViewModel.getSelectedItem()))
        selectedTrack->setSolo(!selectedTrack->isSolo(false));
}

void TracksListViewModel::toggleMute() {
    if (auto selectedTrack = dynamic_cast<tracktion::AudioTrack *>(
            listViewModel.getSelectedItem()))
        selectedTrack->setMute(!selectedTrack->isMuted(false));
}

void TracksListViewModel::setSelectedTrackColour(juce::Colour colour) {
    if (auto selectedTrack = dynamic_cast<tracktion::AudioTrack *>(
            listViewModel.getSelectedItem()))
        selectedTrack->setColour(colour);
}

juce::Colour TracksListViewModel::getSelectedTrackColour() {
    if (auto selectedTrack = dynamic_cast<tracktion::AudioTrack *>(
            listViewModel.getSelectedItem()))
        return selectedTrack->getColour();

    return juce::Colour();
}

void TracksListViewModel::setVideoPosition(tracktion::TimePosition timePosition,
                                           bool forceJump) {
    auto time = timePosition.inSeconds();
    if (time - camera.getCenter() > camera.getCenterOffsetLimit())
        camera.setCenter(time - camera.getCenterOffsetLimit());

    if (camera.getCenter() - time > camera.getCenterOffsetLimit())
        if (time > camera.getScope() / 2.0 - camera.getCenterOffsetLimit())
            camera.setCenter(camera.getCenterOffsetLimit() + time);

    if (time == 0.0)
        camera.setCenter(camera.getScope() / 2.0);
}

void TracksListViewModel::undo() {
    if (edit.getUndoManager().canUndo())
        edit.getUndoManager().undo();
}

void TracksListViewModel::handleAsyncUpdate() {
    if (compareAndReset(shouldUpdateTracksViewType))
        listeners.call([this](Listener &l) {
            l.tracksViewTypeChanged(getTracksViewType());
        });

    if (compareAndReset(shouldUpdateLooping))
        listeners.call([this](Listener &l) {
            l.loopingChanged(edit.getTransport().looping.get());
        });

    if (compareAndReset(shouldUpdateSolo))
        if (auto selectedTrack = dynamic_cast<tracktion::AudioTrack *>(
                listViewModel.getSelectedItem()))
            listeners.call([selectedTrack](Listener &l) {
                l.soloStateChanged(selectedTrack->isSolo(false));
            });

    if (compareAndReset(shouldUpdateMute))
        if (auto selectedTrack = dynamic_cast<tracktion::AudioTrack *>(
                listViewModel.getSelectedItem()))
            listeners.call([selectedTrack](Listener &l) {
                l.muteStateChanged(selectedTrack->isMuted(false));
            });
}

void TracksListViewModel::selectedIndexChanged(int newIndex) {
    for (auto instance : edit.getAllInputDevices()) {
        if (instance->getInputDevice().getDeviceType() ==
            tracktion::InputDevice::physicalMidiDevice) {
            if (auto selectedTrack = dynamic_cast<tracktion::AudioTrack *>(
                    listViewModel.getSelectedItem())) {
                instance->setTargetTrack(*selectedTrack, 0, true);
                instance->setRecordingEnabled(*selectedTrack, true);
            }
        }
    }

    edit.restartPlayback();
}

void TracksListViewModel::changeListenerCallback(juce::ChangeBroadcaster *) {
    if (edit.getTransport().isRecording()) {
        listeners.call([this](Listener &l) {
            l.isRecordingChanged(edit.getTransport().isRecording());
        });
        listeners.call([](Listener &l) { l.isPlayingChanged(false); });

    } else if (edit.getTransport().isPlaying()) {
        listeners.call([](Listener &l) { l.isRecordingChanged(false); });
        listeners.call([](Listener &l) { l.isPlayingChanged(true); });

    } else {
        listeners.call([](Listener &l) { l.isRecordingChanged(false); });
        listeners.call([](Listener &l) { l.isPlayingChanged(false); });
    }
}

void TracksListViewModel::valueTreePropertyChanged(
    juce::ValueTree &treeWhosePropertyHasChanged,
    const juce::Identifier &property) {
    if (treeWhosePropertyHasChanged == state)
        if (property == IDs::tracksListViewType)
            markAndUpdate(shouldUpdateTracksViewType);

    if (treeWhosePropertyHasChanged == edit.getTransport().state)
        if (property == tracktion::IDs::looping)
            markAndUpdate(shouldUpdateLooping);

    if (tracktion::TrackList::isTrack(treeWhosePropertyHasChanged))
        if (property == tracktion::IDs::solo)
            markAndUpdate(shouldUpdateSolo);

    if (tracktion::TrackList::isTrack(treeWhosePropertyHasChanged))
        if (property == tracktion::IDs::mute)
            markAndUpdate(shouldUpdateMute);
}

void TracksListViewModel::addListener(Listener *l) {
    listeners.add(l);
    l->isRecordingChanged(edit.getTransport().isRecording());
    l->isPlayingChanged(edit.getTransport().isPlaying());
    l->tracksViewTypeChanged(getTracksViewType());
    l->loopingChanged(edit.getTransport().looping.get());

    if (auto selectedTrack = dynamic_cast<tracktion::AudioTrack *>(
            listViewModel.getSelectedItem())) {
        l->soloStateChanged(selectedTrack->isSolo(false));
        l->muteStateChanged(selectedTrack->isMuted(false));
    }
}

void TracksListViewModel::removeListener(Listener *l) { listeners.remove(l); }

} // namespace app_view_models