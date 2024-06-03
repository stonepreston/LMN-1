#include "MidiCommandManager.h"
#include <typeinfo>
namespace app_services {

MidiCommandManager::MidiCommandManager(tracktion::Engine &e) : engine(e) {
    // need  to listen to midi events to pass to the midi command manager
    // to do this we need to call the addMidiInputDeviceCallback method
    // on the JUCE deviceManager (not the tracktion wrapper)
    // also we will enable the device if its disabled
    auto &juceDeviceManager = engine.getDeviceManager().deviceManager;
    auto list = juce::MidiInput::getAvailableDevices();
    for (const auto &midiDevice : list) {
        juce::Logger::writeToLog("enabling juce midi device: " +
                                 midiDevice.name);
        juceDeviceManager.setMidiInputDeviceEnabled(midiDevice.identifier,
                                                    true);

        juce::Logger::writeToLog("adding callback for juce midi device: " +
                                 midiDevice.name);
        juceDeviceManager.addMidiInputDeviceCallback(midiDevice.identifier,
                                                     this);
    }
}

MidiCommandManager::~MidiCommandManager() {
    auto &juceDeviceManager = engine.getDeviceManager().deviceManager;
    auto list = juce::MidiInput::getAvailableDevices();
    for (const auto &midiDevice : list) {
        juceDeviceManager.removeMidiInputDeviceCallback(midiDevice.identifier,
                                                        this);
    }
}

void MidiCommandManager::setFocusedComponent(juce::Component *c) {
    jassert(c != nullptr);
    focusedComponent = c;
}

juce::Component *MidiCommandManager::getFocusedComponent() {
    return focusedComponent;
}

void MidiCommandManager::handleIncomingMidiMessage(
    juce::MidiInput *source, const juce::MidiMessage &message) {
    (new IncomingMessageCallback(*this, message, source->getName()))->post();
}

void MidiCommandManager::midiMessageReceived(const juce::MidiMessage &message,
                                             const juce::String &source) {
    juce::Logger::writeToLog(getMidiMessageDescription(message));

    if (message.isNoteOn()) {
        if (auto listener = dynamic_cast<Listener *>(focusedComponent))
            listener->noteOnPressed(message.getNoteNumber());
    }

    if (message.isController()) {
        listeners.call([message](Listener &l) {
            l.controllerEventReceived(message.getControllerNumber(),
                                      message.getControllerValue());
        });

        switch (message.getControllerNumber()) {
        case ENCODER_1:
            if (auto listener = dynamic_cast<Listener *>(focusedComponent)) {
                if (message.getControllerValue() == 1)
                    listener->encoder1Increased();

                if (message.getControllerValue() == 127)
                    listener->encoder1Decreased();
            }

            break;

        case ENCODER_2:
            if (auto listener = dynamic_cast<Listener *>(focusedComponent)) {
                if (message.getControllerValue() == 1)
                    listener->encoder2Increased();

                if (message.getControllerValue() == 127)
                    listener->encoder2Decreased();
            }

            break;

        case ENCODER_3:
            if (auto listener = dynamic_cast<Listener *>(focusedComponent)) {
                if (message.getControllerValue() == 1)
                    listener->encoder3Increased();

                if (message.getControllerValue() == 127)
                    listener->encoder3Decreased();
            }

            break;

        case ENCODER_4:
            if (auto listener = dynamic_cast<Listener *>(focusedComponent)) {
                if (message.getControllerValue() == 1)
                    listener->encoder4Increased();

                if (message.getControllerValue() == 127)
                    listener->encoder4Decreased();
            }

            break;

        case ENCODER_1_BUTTON:
            if (auto listener = dynamic_cast<Listener *>(focusedComponent)) {
                if (message.getControllerValue() == 127)
                    listener->encoder1ButtonPressed();

                if (message.getControllerValue() == 0)
                    listener->encoder1ButtonReleased();
            }

            break;

        case ENCODER_2_BUTTON:
            if (auto listener = dynamic_cast<Listener *>(focusedComponent)) {
                if (message.getControllerValue() == 127)
                    listener->encoder2ButtonPressed();

                if (message.getControllerValue() == 0)
                    listener->encoder2ButtonReleased();
            }

            break;

        case ENCODER_3_BUTTON:
            if (auto listener = dynamic_cast<Listener *>(focusedComponent)) {
                if (message.getControllerValue() == 127)
                    listener->encoder3ButtonPressed();

                if (message.getControllerValue() == 0)
                    listener->encoder3ButtonReleased();
            }

            break;

        case ENCODER_4_BUTTON:
            if (auto listener = dynamic_cast<Listener *>(focusedComponent)) {
                if (message.getControllerValue() == 127)
                    listener->encoder4ButtonPressed();

                if (message.getControllerValue() == 0)
                    listener->encoder4ButtonReleased();
            }

            break;

        case TRACKS_BUTTON:
            // This should be called for all listeners, not just the currently
            // focused component
            if (message.getControllerValue() == 127)
                listeners.call([](Listener &l) { l.tracksButtonPressed(); });

            if (message.getControllerValue() == 0)
                listeners.call([](Listener &l) { l.tracksButtonReleased(); });

            break;

        case MIXER_BUTTON:
            // This should be called for all listeners, not just the currently
            // focused component
            if (message.getControllerValue() == 127)
                listeners.call([](Listener &l) { l.mixerButtonPressed(); });

            if (message.getControllerValue() == 0)
                listeners.call([](Listener &l) { l.mixerButtonReleased(); });

            break;

        case PLUGINS_BUTTON:
            // This should be called for all listeners, not just the currently
            // focused component
            if (message.getControllerValue() == 127)
                listeners.call([](Listener &l) { l.pluginsButtonPressed(); });

            if (message.getControllerValue() == 0)
                listeners.call([](Listener &l) { l.pluginsButtonReleased(); });

            break;

        case MODIFIERS_BUTTON:
            // This should be called for all listeners, not just the currently
            // focused component
            if (message.getControllerValue() == 127)
                listeners.call([](Listener &l) { l.modifiersButtonPressed(); });

            if (message.getControllerValue() == 0)
                listeners.call(
                    [](Listener &l) { l.modifiersButtonReleased(); });

            break;

        case SETTINGS_BUTTON:
            // This should be called for all listeners, not just the currently
            // focused component
            if (message.getControllerValue() == 127)
                listeners.call([](Listener &l) { l.settingsButtonPressed(); });

            if (message.getControllerValue() == 0)
                listeners.call([](Listener &l) { l.settingsButtonReleased(); });

            break;

        case TEMPO_BUTTON:
            // This should be called for all listeners, not just the currently
            // focused component
            if (message.getControllerValue() == 127)
                listeners.call(
                    [](Listener &l) { l.tempoSettingsButtonPressed(); });

            if (message.getControllerValue() == 0)
                listeners.call(
                    [](Listener &l) { l.tempoSettingsButtonReleased(); });

            break;

        case SAVE_BUTTON:
            // This should be called for all listeners, not just the currently
            // focused component

            if (isControlDown) {
                if (message.getControllerValue() == 127)
                    listeners.call(
                        [](Listener &l) { l.renderButtonPressed(); });

                if (message.getControllerValue() == 0)
                    listeners.call(
                        [](Listener &l) { l.renderButtonReleased(); });
            } else {
                if (message.getControllerValue() == 127)
                    listeners.call([](Listener &l) { l.saveButtonPressed(); });

                if (message.getControllerValue() == 0)
                    listeners.call([](Listener &l) { l.saveButtonReleased(); });
            }

            break;

        case RECORD_BUTTON:
            if (auto listener = dynamic_cast<Listener *>(focusedComponent)) {
                if (message.getControllerValue() == 127)
                    listener->recordButtonPressed();

                if (message.getControllerValue() == 0)
                    listener->recordButtonReleased();
            }

            break;

        case PLAY_BUTTON:
            if (auto listener = dynamic_cast<Listener *>(focusedComponent)) {
                if (message.getControllerValue() == 127)
                    listener->playButtonPressed();

                if (message.getControllerValue() == 0)
                    listener->playButtonReleased();
            }

            break;

        case STOP_BUTTON:
            if (auto listener = dynamic_cast<Listener *>(focusedComponent)) {
                if (message.getControllerValue() == 127)
                    listener->stopButtonPressed();

                if (message.getControllerValue() == 0)
                    listener->stopButtonReleased();
            }

            break;

        case SEQUENCERS_BUTTON:
            // This should be called for all listeners, not just the currently
            // focused component
            if (message.getControllerValue() == 127)
                listeners.call(
                    [](Listener &l) { l.sequencersButtonPressed(); });

            if (message.getControllerValue() == 0)
                listeners.call(
                    [](Listener &l) { l.sequencersButtonReleased(); });

            break;

        case CUT_BUTTON:
            if (auto listener = dynamic_cast<Listener *>(focusedComponent)) {
                if (message.getControllerValue() == 127)
                    listener->cutButtonPressed();

                if (message.getControllerValue() == 0)
                    listener->cutButtonReleased();
            }

            break;

        case PASTE_BUTTON:
            if (auto listener = dynamic_cast<Listener *>(focusedComponent)) {
                if (message.getControllerValue() == 127)
                    listener->pasteButtonPressed();

                if (message.getControllerValue() == 0)
                    listener->pasteButtonReleased();
            }

            break;

        case SLICE_BUTTON:
            if (auto listener = dynamic_cast<Listener *>(focusedComponent)) {
                if (message.getControllerValue() == 127)
                    listener->sliceButtonPressed();

                if (message.getControllerValue() == 0)
                    listener->sliceButtonReleased();
            }

            break;

        case CONTROL_BUTTON:
            if (message.getControllerValue() == 127) {
                isControlDown = true;
                listeners.call([](Listener &l) { l.controlButtonPressed(); });
            }

            if (message.getControllerValue() == 0) {
                isControlDown = false;
                listeners.call([](Listener &l) { l.controlButtonReleased(); });
            }

            break;

        case LOOP_IN_BUTTON:
            if (message.getControllerValue() == 127)
                listeners.call([](Listener &l) { l.loopInButtonPressed(); });

            if (message.getControllerValue() == 0)
                listeners.call([](Listener &l) { l.loopInButtonReleased(); });

            break;

        case LOOP_OUT_BUTTON:
            if (message.getControllerValue() == 127)
                listeners.call([](Listener &l) { l.loopOutButtonPressed(); });

            if (message.getControllerValue() == 0)
                listeners.call([](Listener &l) { l.loopOutButtonReleased(); });

            break;

        case LOOP_BUTTON:
            if (auto listener = dynamic_cast<Listener *>(focusedComponent)) {
                if (isControlDown) {
                    if (message.getControllerValue() == 127)
                        listener->undoButtonPressed();

                    if (message.getControllerValue() == 0)
                        listener->undoButtonReleased();

                } else {
                    if (message.getControllerValue() == 127)
                        listener->loopButtonPressed();

                    if (message.getControllerValue() == 0)
                        listener->loopButtonReleased();
                }
            }

            break;

        case UNDO_BUTTON:
            if (auto listener = dynamic_cast<Listener *>(focusedComponent)) {
                if (message.getControllerValue() == 127)
                    listener->undoButtonPressed();

                if (message.getControllerValue() == 0)
                    listener->undoButtonReleased();
            }

            break;

        case OCTAVE_CHANGE:
            // This should be called for all listeners, not just the currently
            // focused component controller message values will be between 0 and
            // 8, 4 is the "home" octave (we will display 0) 0 is min octave
            // (-4) 8 is max octave (+4)
            listeners.call([message](Listener &l) {
                l.octaveChanged(message.getControllerValue() - 4);
            });

            break;

        case PLUS_BUTTON:
            if (message.getControllerValue() == 127) {
                isPlusDown = true;
                listeners.call([](Listener &l) { l.plusButtonPressed(); });
            }

            if (message.getControllerValue() == 0) {
                isPlusDown = false;
                listeners.call([](Listener &l) { l.plusButtonReleased(); });
            }

            break;

        case MINUS_BUTTON:
            if (message.getControllerValue() == 127) {
                isMinusDown = true;
                listeners.call([](Listener &l) { l.minusButtonPressed(); });
            }

            if (message.getControllerValue() == 0) {
                isMinusDown = false;
                listeners.call([](Listener &l) { l.minusButtonReleased(); });
            }

            break;

        default:
            break;
        }
    }
}

juce::String
MidiCommandManager::getMidiMessageDescription(const juce::MidiMessage &m) {
    if (m.isNoteOn())
        return "Note on " + juce::MidiMessage::getMidiNoteName(
                                m.getNoteNumber(), true, true, 3);
    if (m.isNoteOff())
        return "Note off " + juce::MidiMessage::getMidiNoteName(
                                 m.getNoteNumber(), true, true, 3);
    if (m.isProgramChange())
        return "Program change " + juce::String(m.getProgramChangeNumber());
    if (m.isPitchWheel())
        return "Pitch wheel " + juce::String(m.getPitchWheelValue());
    if (m.isAftertouch())
        return "After touch " +
               juce::MidiMessage::getMidiNoteName(m.getNoteNumber(), true, true,
                                                  3) +
               ": " + juce::String(m.getAfterTouchValue());
    if (m.isChannelPressure())
        return "Channel pressure " + juce::String(m.getChannelPressureValue());
    if (m.isAllNotesOff())
        return "All notes off";
    if (m.isAllSoundOff())
        return "All sound off";
    if (m.isMetaEvent())
        return "Meta event";

    if (m.isController()) {
        juce::String name = "[" + juce::String(m.getControllerNumber()) + "]";

        return "Controller " + name + ": " +
               juce::String(m.getControllerValue());
    }

    return juce::String::toHexString(m.getRawData(), m.getRawDataSize());
}

void MidiCommandManager::addListener(MidiCommandManager::Listener *l) {
    listeners.add(l);
}

void MidiCommandManager::removeListener(MidiCommandManager::Listener *l) {
    listeners.remove(l);
}

} // namespace app_services