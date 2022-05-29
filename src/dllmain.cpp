#include "includes.h"
#include <random>
#include <algorithm>
#include <fstream>
#include "extensions2.h"
#include "uwuifier.h"

bool enabled = true;
bool enabledInLevels = false;

bool ignoreUwuifying = false;
void (__thiscall* CCLabelBMFont_setString)(CCLabelBMFont* self, const char *newString, bool needUpdateLabel);
void __fastcall CCLabelBMFont_setString_H(CCLabelBMFont* self, void*, const char *newString, bool needUpdateLabel) {
    auto parent = self->getParent();
    // same as comment on the hook below
    if(!enabled || !enabledInLevels && (ignoreUwuifying || parent != nullptr && typeid(*parent) == typeid(gd::GameObject))) {
        CCLabelBMFont_setString(self, newString, needUpdateLabel);
        return;
    }

    uwuifier::seedFrom(self);
    CCLabelBMFont_setString(self, uwuifier::uwuify(newString).c_str(), needUpdateLabel);
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
void MH_CALL extChangedPeriodToExclamationChance(MegaHackExt::Spinner* self, double value) { uwuifier::getSettings().periodToExclamationChance = value / 100; }
void MH_CALL extChangedStutterChance(MegaHackExt::Spinner* self, double value) { uwuifier::getSettings().stutterChance = value / 100; }
void MH_CALL extChangedPresuffixChance(MegaHackExt::Spinner* self, double value) { uwuifier::getSettings().presuffixChance = value / 100; }
void MH_CALL extChangedSuffixChance(MegaHackExt::Spinner* self, double value) { uwuifier::getSettings().suffixChance = value / 100; }
void MH_CALL extChangedDuplicateCharactersChance(MegaHackExt::Spinner* self, double value) { uwuifier::getSettings().duplicateCharactersChance = value / 100; }
void MH_CALL extChangedDuplicateCharactersAmount(MegaHackExt::Spinner* self, double value) { uwuifier::getSettings().duplicateCharactersAmount = (int)value; }

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

    auto ext = MegaHackExt::Window::Create(uwuifier::uwuify("uwuifier").c_str());

    auto duplicateCharactersAmountUi = MegaHackExt::Spinner::Create("", "");
    duplicateCharactersAmountUi->setCallback(extChangedDuplicateCharactersAmount);
    duplicateCharactersAmountUi->set(uwuifier::getSettings().duplicateCharactersAmount);
    ext->add(duplicateCharactersAmountUi);
    auto duplicateCharactersAmountLabel = MegaHackExt::Label::Create(uwuifier::uwuify("Duplicate Characters Amount:").c_str());
    ext->add(duplicateCharactersAmountLabel);

    auto duplicateCharactersChanceUi = MegaHackExt::Spinner::Create("", uwuifier::uwuify("%").c_str());
    duplicateCharactersChanceUi->setCallback(extChangedDuplicateCharactersChance);
    duplicateCharactersChanceUi->set(uwuifier::getSettings().duplicateCharactersChance * 100.0);
    ext->add(duplicateCharactersChanceUi);
    auto duplicateCharactersChanceLabel = MegaHackExt::Label::Create(uwuifier::uwuify("Duplicate Characters Chance:").c_str());
    ext->add(duplicateCharactersChanceLabel);

    auto suffixChanceUi = MegaHackExt::Spinner::Create("", uwuifier::uwuify("%").c_str());
    suffixChanceUi->setCallback(extChangedSuffixChance);
    suffixChanceUi->set(uwuifier::getSettings().suffixChance * 100.0);
    ext->add(suffixChanceUi);
    auto suffixChanceLabel = MegaHackExt::Label::Create(uwuifier::uwuify("Suffix Chance:").c_str());
    ext->add(suffixChanceLabel);

    auto presuffixChanceUi = MegaHackExt::Spinner::Create("", uwuifier::uwuify("%").c_str());
    presuffixChanceUi->setCallback(extChangedPresuffixChance);
    presuffixChanceUi->set(uwuifier::getSettings().presuffixChance * 100.0);
    ext->add(presuffixChanceUi);
    auto presuffixChanceLabel = MegaHackExt::Label::Create(uwuifier::uwuify("Presuffix Chance:").c_str());
    ext->add(presuffixChanceLabel);

    auto stutterChanceUi = MegaHackExt::Spinner::Create("", uwuifier::uwuify("%").c_str());
    stutterChanceUi->setCallback(extChangedStutterChance);
    stutterChanceUi->set(uwuifier::getSettings().stutterChance * 100.0);
    ext->add(stutterChanceUi);
    auto stutterChanceLabel = MegaHackExt::Label::Create(uwuifier::uwuify("Stutter Chance:").c_str());
    ext->add(stutterChanceLabel);

    auto periodToExclamationChanceUi = MegaHackExt::Spinner::Create("", uwuifier::uwuify("%").c_str());
    periodToExclamationChanceUi->setCallback(extChangedPeriodToExclamationChance);
    periodToExclamationChanceUi->set(uwuifier::getSettings().periodToExclamationChance * 100.0);
    ext->add(periodToExclamationChanceUi);
    auto periodToExclamationChanceLabel = MegaHackExt::Label::Create(uwuifier::uwuify("Period To Exclamation Chance:").c_str());
    ext->add(periodToExclamationChanceLabel);

    auto enabledInLevelsUi = MegaHackExt::CheckBox::Create(uwuifier::uwuify("In Levels").c_str());
    enabledInLevelsUi->setCallback(extToggledEnabledInLevelsCallback);
    enabledInLevelsUi->set(enabledInLevels);

    auto enabledUi = MegaHackExt::CheckBox::Create(uwuifier::uwuify("Enabled").c_str());
    enabledUi->setCallback(extToggledEnabledCallback);
    enabledUi->set(enabled);

    ext->add(MegaHackExt::HorizontalLayout::Create(enabledUi, enabledInLevelsUi));

    MegaHackExt::Client::commit(ext);

    return 0;
}

BOOL APIENTRY DllMain(HMODULE handle, DWORD reason, LPVOID reserved) {
    if(reason == DLL_PROCESS_ATTACH) {
        CreateThread(0, 0x100, mainThread, handle, 0, 0);
    }
    return TRUE;
}