#pragma once

namespace uwuifier {
    class Settings {
    public:
        double periodToExclamationChance = 0.2;
        double stutterChance = 0.1;
        double presuffixChance = 0.1;
        double suffixChance = 0.3;
        double duplicateCharactersChance = 0.4;
        int duplicateCharactersAmount = 3;
    };

    Settings& getSettings();
    void resetSettings();

    void seedFrom(const void* ptr);
    std::string uwuify(std::string text);
}
