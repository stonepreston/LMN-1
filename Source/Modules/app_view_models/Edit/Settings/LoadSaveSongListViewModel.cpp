#include "LoadSaveSongListViewModel.h"

namespace app_view_models {

LoadSaveSongListViewModel::LoadSaveSongListViewModel(
    tracktion::Edit &e, juce::AudioDeviceManager &dm,
    const juce::String &appName)
    : deviceManager(dm), state(e.state.getOrCreateChildWithName(
                             IDs::LOAD_SAVE_SONG_VIEW_STATE, nullptr)),
      itemListState(state, songNames.size()) {

    auto userAppDataDirectory = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory);
    juce::File savedDirectory =
        userAppDataDirectory.getChildFile(appName)
            .getChildFile("saved");
    
    loadSongList(savedDirectory);
    itemListState.addListener(this);
}

LoadSaveSongListViewModel::~LoadSaveSongListViewModel() {
    itemListState.removeListener(this);
}

juce::StringArray LoadSaveSongListViewModel::getItemNames() {
    return songNames;
}

juce::String LoadSaveSongListViewModel::getSelectedItem() {
    return songNames[itemListState.getSelectedItemIndex()];
}

void LoadSaveSongListViewModel::selectedIndexChanged(int newIndex) {
    // Aquí puedes añadir la lógica que necesites cuando cambie el índice
    // seleccionado
}

void LoadSaveSongListViewModel::loadSongList(const juce::File &directory) {
    songNames.clear();
    songNames.add("Add");   

    juce::Array<juce::File> songFiles =
        directory.findChildFiles(juce::File::findFiles, false, "*.xml");

    for (const auto &file : songFiles) {
        songNames.add(file.getFileNameWithoutExtension());
    }

    itemListState.listSize = songNames.size();
    itemListState.setSelectedItemIndex(
        0); // Opcional: Resetear al primer ítem o cualquier otra lógica que
            // necesites
    itemListState.addListener(this);
}
void saveEditState(const tracktion::engine::Edit &edit,
                   const juce::File &file) {
    // Serializar el estado del Edit a XML
    auto xml = edit.state.createXml();

    // Guardar el XML en un archivo
    if (xml) {
        if (!xml->writeTo(file)) {
            // Manejar el error si la escritura al archivo falla
            DBG("Failed to write Edit state to file.");
        }
    } else {
        // Manejar el error si la serialización falla
        DBG("Failed to create XML from Edit state.");
    }
}
void loadEditState(tracktion::engine::Edit &edit, const juce::File &file) {
    // Leer el XML desde el archivo
    auto xml = juce::XmlDocument::parse(file);

    if (xml) {
        // Deserializar el XML para restaurar el estado del Edit
        edit.state = juce::ValueTree::fromXml(*xml);
    } else {
        // Manejar el error si la lectura del archivo falla
        DBG("Failed to read Edit state from file.");
    }
}

} // namespace app_view_models
