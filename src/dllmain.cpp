#include "includes.h"
#include <random>
#include <algorithm>
#include <fstream>
#include "extensions2.h"

bool enabled = true;
bool enabledInLevels = false;
// multiplied by 10000
unsigned int periodToExclamationChance = 2000;
int stutterChance = 1000;
int presuffixChance = 1000;
int suffixChance = 2000;

static std::unordered_map<char, char> const SINGLE_REPLACE = {
    { 'l', 'w' },
    { 'r', 'w' }
};

static std::unordered_map<std::string, std::string> const REPLACE = {
    { " you ", " uwu " },
    { " no ", " nu " },
    { " na", " nya" },
    { " ne", " nye" },
    { " ni", " nyi" },
    { " no", " nyo" },
    { " nu", " nyu" },
    { "te", "twe" },
    { "da", "dwa" },
    { "ke", "kwe" },
    { "qe", "qwe" },
    { "je", "jwe" },
    { "ne", "nwe" },
    { "na", "nwa" },
    { "si", "swi" },
    { "so", "swo" },
    { "mi", "mwi" },
    { "co", "cwo" },
    { "mo", "mwo" },
    { "ba", "bwa" },
    { "pow", "paw" }
};

static std::vector<std::string> const PRESUFFIXES = {
    "~"
};

static std::vector<std::string> const SUFFIXES = {
    " :D",
    " xD",
    " :P",
    " ;3",
    " <{^v^}>",
    " ^-^",
    " x3",
    " x3",
    " rawr",
    " rawr x3",
    " owo",
    " uwu",
    " -.-",
    " >w<",
    " :3",
    " XD",
    " nyaa~~",
    " >_<",
    " :flushed:",
    " ^^",
    " ^^;;"
};

void replaceAll(std::string& inout, std::string_view what, std::string_view with) {
    std::size_t count{};
    for(std::string::size_type pos{}; inout.npos != (pos = inout.find(what.data(), pos, what.length())); pos += with.length(), ++count) {
        inout.replace(pos, what.length(), with.data(), with.length());
    }
}

std::random_device::result_type rd = std::random_device()();
template <class T>
inline void hash_combine(std::size_t& seed, const T& v) {
    std::hash<T> hasher;
    seed ^= hasher(v) + rd + (seed << 6) + (seed >> 2);
}

inline bool getChance(int chance) { return std::rand() % 10000 < chance; }

std::string newString = std::string();
const char* uwuify(const char* originalString) {
    size_t origLength = strlen(originalString);
    bool isEmpty = true;
    for(size_t i = 0; i < origLength; i++) {
        if(!isspace(originalString[i])) isEmpty = false;
    }
    if(isEmpty) return originalString;

    newString.clear();

    // copy and replace . with !
    newString.append(" ");
    for(size_t i = 0; i < origLength; i++) {
        char prevChar = i == 0 ? ' ' : tolower(originalString[i - 1]);
        char currChar = tolower(originalString[i]);
        char nextChar = i == origLength - 1 ? ' ' : tolower(originalString[i + 1]);

        if(currChar == '.' && nextChar == ' ' && getChance(periodToExclamationChance)) currChar = '!';
        newString += currChar;
    }
    newString.append(" ");

    // replacements
    for(size_t i = 1; i < newString.size() - 1; i++) {
        char prevChar = newString[i - 1];
        char currChar = newString[i];
        char nextChar = newString[i + 1];

        if(!isalpha(prevChar) && !isalpha(nextChar) || !SINGLE_REPLACE.contains(currChar)) continue;
        newString[i] = SINGLE_REPLACE.at(currChar);
    }
    for(auto elem : REPLACE) {
        replaceAll(newString, elem.first, elem.second);
    }

    // stutter and suffixes
    for(size_t i = 1; i < newString.size(); i++) {
        bool end = i == newString.size() - 1;

        char prevChar = newString[i - 1];
        char currChar = newString[i];
        char nextChar = end ? ' ' : newString[i + 1];

        bool stutter = prevChar == ' ' && isalpha(currChar) && getChance(stutterChance);
        if(stutter) {
            newString.insert(i, 1, '-');
            newString.insert(i, 1, currChar);
        }

        int suffixLength = 0;
        if(currChar == ' ' && (prevChar == '.' || prevChar == '!' || prevChar == ',' || nextChar == '-' || end)) {
            if(getChance(suffixChance) && (!end || isalnum(prevChar))) {
                auto suffix = SUFFIXES[std::rand() % SUFFIXES.size()];
                newString.insert(i, suffix);
                suffixLength += suffix.size();
            }
            if(getChance(presuffixChance)) {
                auto presuffix = PRESUFFIXES[std::rand() % PRESUFFIXES.size()];
                newString.insert(prevChar != ',' || end ? i : i - 1, presuffix);
                suffixLength += presuffix.size();
            }
        }

        if(stutter) i += 2;
        i += suffixLength;
    }

    newString = newString.substr(1, newString.size() - 2);
    return newString.c_str();
}

bool ignoreUwuifying = false;
void (__thiscall* CCLabelBMFont_setString)(CCLabelBMFont* self, const char *newString, bool needUpdateLabel);
void __fastcall CCLabelBMFont_setString_H(CCLabelBMFont* self, void*, const char *newString, bool needUpdateLabel) {
    auto parent = self->getParent();
    // same as comment on the hook below
    if(!enabled || !enabledInLevels && (ignoreUwuifying || parent != nullptr && typeid(*parent) == typeid(gd::GameObject))) {
        CCLabelBMFont_setString(self, newString, needUpdateLabel);
        return;
    }

    auto seed = (unsigned int)rd;
    hash_combine(seed, self);
    std::srand(seed);
    CCLabelBMFont_setString(self, uwuify(newString), needUpdateLabel);
}

// don't uwuify the levels themselves, or they may break xd
void (__thiscall* GameObject_updateTextObject)(gd::GameObject* self, char param_1, std::string param_2);
void __fastcall GameObject_updateTextObject_H(gd::GameObject* self, void*, char param_1, std::string param_2) {
    ignoreUwuifying = true;
    GameObject_updateTextObject(self, param_1, param_2);
    ignoreUwuifying = false;
}

gd::GameObject* (__thiscall* LevelEditorLayer_addObjectFromString)(gd::LevelEditorLayer* self, std::string str);
gd::GameObject* __fastcall LevelEditorLayer_addObjectFromString_H(gd::LevelEditorLayer* self, void*, std::string str) {
    ignoreUwuifying = true;
    auto ret = LevelEditorLayer_addObjectFromString(self, str);
    ignoreUwuifying = false;
    return ret;
}

gd::GameObject* (__thiscall* LevelEditorLayer_createObject)(gd::LevelEditorLayer* self, int id, cocos2d::CCPoint position, bool undo);
gd::GameObject* __fastcall LevelEditorLayer_createObject_H(gd::LevelEditorLayer* self, void*, int id, cocos2d::CCPoint position, bool undo) {
    ignoreUwuifying = true;
    auto ret = LevelEditorLayer_createObject(self, id, position, undo);
    ignoreUwuifying = false;
    return ret;
}

void MH_CALL extToggledEnabledCallback(MegaHackExt::CheckBox* self, bool toggled) { enabled = toggled; }
void MH_CALL extToggledEnabledInLevelsCallback(MegaHackExt::CheckBox* self, bool toggled) { enabledInLevels = toggled; }

// percents
void MH_CALL extChangedPeriodToExclamationChance(MegaHackExt::Spinner* self, double value) { periodToExclamationChance = value * 100; }
void MH_CALL extChangedStutterChance(MegaHackExt::Spinner* self, double value) { stutterChance = value * 100; }
void MH_CALL extChangedPresuffixChance(MegaHackExt::Spinner* self, double value) { presuffixChance = value * 100; }
void MH_CALL extChangedSuffixChance(MegaHackExt::Spinner* self, double value) { suffixChance = value * 100; }

DWORD WINAPI mainThread(void* hModule) {
    MH_Initialize();

    auto base = reinterpret_cast<uintptr_t>(GetModuleHandle(0));
    auto cocos2dBase = reinterpret_cast<uintptr_t>(GetModuleHandle("libcocos2d.dll"));

    MH_CreateHook(
        reinterpret_cast<void*>(cocos2dBase + 0x9fb60),
        CCLabelBMFont_setString_H,
        reinterpret_cast<void**>(&CCLabelBMFont_setString)
    );

    MH_CreateHook(
        reinterpret_cast<void*>(base + 0xcfc60),
        GameObject_updateTextObject_H,
        reinterpret_cast<void**>(&GameObject_updateTextObject)
    );

    MH_CreateHook(
        reinterpret_cast<void*>(base + 0x160c80),
        LevelEditorLayer_addObjectFromString_H,
        reinterpret_cast<void**>(&LevelEditorLayer_addObjectFromString)
    );

    MH_CreateHook(
        reinterpret_cast<void*>(base + 0x160d70),
        LevelEditorLayer_createObject_H,
        reinterpret_cast<void**>(&LevelEditorLayer_createObject)
    );

    MH_EnableHook(MH_ALL_HOOKS);

    auto ext = MegaHackExt::Window::Create(uwuify("uwuifier"));

    auto suffixChanceUi = MegaHackExt::Spinner::Create("", uwuify("%"));
    suffixChanceUi->setCallback(extChangedSuffixChance);
    suffixChanceUi->set(suffixChance / 100.0);
    ext->add(suffixChanceUi);
    auto suffixChanceLabel = MegaHackExt::Label::Create(uwuify("Suffix Chance:"));
    ext->add(suffixChanceLabel);

    auto presuffixChanceUi = MegaHackExt::Spinner::Create("", uwuify("%"));
    presuffixChanceUi->setCallback(extChangedPresuffixChance);
    presuffixChanceUi->set(presuffixChance / 100.0);
    ext->add(presuffixChanceUi);
    auto presuffixChanceLabel = MegaHackExt::Label::Create(uwuify("Presuffix Chance:"));
    ext->add(presuffixChanceLabel);

    auto stutterChanceUi = MegaHackExt::Spinner::Create("", uwuify("%"));
    stutterChanceUi->setCallback(extChangedStutterChance);
    stutterChanceUi->set(stutterChance / 100.0);
    ext->add(stutterChanceUi);
    auto stutterChanceLabel = MegaHackExt::Label::Create(uwuify("Stutter Chance:"));
    ext->add(stutterChanceLabel);

    auto periodToExclamationChanceUi = MegaHackExt::Spinner::Create("", uwuify("%"));
    periodToExclamationChanceUi->setCallback(extChangedPeriodToExclamationChance);
    periodToExclamationChanceUi->set(periodToExclamationChance / 100.0);
    ext->add(periodToExclamationChanceUi);
    auto periodToExclamationChanceLabel = MegaHackExt::Label::Create(uwuify("Period To Exclamation Chance:"));
    ext->add(periodToExclamationChanceLabel);

    auto enabledInLevelsUi = MegaHackExt::CheckBox::Create(uwuify("In Levels"));
    enabledInLevelsUi->setCallback(extToggledEnabledInLevelsCallback);
    enabledInLevelsUi->set(enabledInLevels);

    auto enabledUi = MegaHackExt::CheckBox::Create(uwuify("Enabled"));
    enabledUi->setCallback(extToggledEnabledCallback);
    enabledUi->set(enabled);

    ext->add(MegaHackExt::HorizontalLayout::Create(enabledUi, enabledInLevelsUi));

    MegaHackExt::Client::commit(ext);

    return 0;
}

BOOL APIENTRY DllMain(HMODULE handle, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        CreateThread(0, 0x100, mainThread, handle, 0, 0);
    }
    return TRUE;
}