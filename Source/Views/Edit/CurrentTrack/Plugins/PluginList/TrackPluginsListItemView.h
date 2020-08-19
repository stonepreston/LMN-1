
#include <juce_gui_extra/juce_gui_extra.h>

class TrackPluginsListItemView : public juce::Component
{
public:

    enum ColourIds
    {
        unselectedBackgroundColourId = 0x2000280,
        selectedBackgroundColourId = 0x2000281,
        selectedTextColourId = 0x2000282,
        unselectedTextColourId = 0x2000283
    };

    explicit TrackPluginsListItemView(juce::String title);

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setTitle(juce::String t);
    void setSelected(bool selected);

    void lookAndFeelChanged() override;

private:

    juce::Label titleLabel;
    juce::Colour backgroundColour;
    juce::Colour textColour;
    bool isSelected = false;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackPluginsListItemView)
};
