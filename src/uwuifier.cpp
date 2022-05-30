#include <random>
#include <algorithm>
#include <unordered_map>
#include <boost/regex.hpp>
#include <functional>
#include <format>

#include "uwuifier.h"
namespace uwuifier {
    static Settings _settings = Settings();

    Settings& getSettings() { return _settings; }
    void resetSettings() { _settings = Settings(); }

    std::random_device::result_type rd = std::random_device()();
    std::mt19937 gen = std::mt19937(rd);
    inline bool getChance(double chance) { return std::generate_canonical<double, 10>(gen) < chance; }

    bool isCaps(const std::string& text) {
        if(text.size() <= 1)
            return false;
        for(auto character : text)
            if(islower(character))
                return false;
        return true;
    }

    std::string toLower(const std::string& text) {
        std::string newText = "";
        for(auto character : text)
            newText += tolower(character);
        return newText;
    }

    std::string toUpper(const std::string& text) {
        std::string newText = "";
        for(auto character : text)
            newText += toupper(character);
        return newText;
    }

    bool isNullOrWhiteSpace(std::string str) {
        return std::find_if(str.begin(), str.end(),
                [](unsigned char ch) { return !isspace(ch); }) == str.end();
    }

    static std::vector<std::pair<boost::regex, std::string>> const simpleReplacements = {
        { boost::regex("[lr]"), "w" },
        { boost::regex("n(?=[aeiou])"), "ny" },
        { boost::regex("pow"), "paw" },
        { boost::regex("(?<!w)u(?=[ie])"), "w" },
        { boost::regex("attempt"), "attwempt" },
        { boost::regex("config"), "cwonfig" }
    };

    static std::unordered_map<std::string, std::string> const wordReplacements = {
        { "you", "uwu" },
        { "no", "nu" },
        { "oh", "ow" },
        { "too", "two" },
        { "attempt", "attwempt" },
        { "config", "cwonfig" }
    };

    class SuffixChoice {
    private:
        const char* _text = nullptr;
        std::vector<SuffixChoice> _choices = { };

    public:
        SuffixChoice(const char* text) { _text = text; }
        SuffixChoice(const std::vector<SuffixChoice> choices) { _choices = choices; }

        const char* choose() const {
            if(_choices.size() > 0) {
                std::uniform_int_distribution<int> dist(0, _choices.size() - 1);
                return _choices.at(dist(gen)).choose();
            }

            if(_text != nullptr)
                return _text;

            return "";
        }
    };

    static const SuffixChoice& presuffixes = SuffixChoice(std::vector<SuffixChoice> {
        SuffixChoice("~"),
        SuffixChoice("~~"),
        SuffixChoice(",")
    });

    static const SuffixChoice& suffixes = SuffixChoice(std::vector<SuffixChoice> {
        SuffixChoice(":D"),
        SuffixChoice(std::vector<SuffixChoice> {
            SuffixChoice("xD"),
            SuffixChoice("XD")
        }),
        SuffixChoice(":P"),
        SuffixChoice(";3"),
        SuffixChoice("<{^v^}>"),
        SuffixChoice("^-^"),
        SuffixChoice("x3"),
        SuffixChoice(std::vector<SuffixChoice> {
            SuffixChoice("rawr"),
            SuffixChoice("rawr~"),
            SuffixChoice("rawr~~"),
            SuffixChoice("rawr x3"),
            SuffixChoice("rawr~ x3"),
            SuffixChoice("rawr~~ x3")
        }),
        SuffixChoice(std::vector<SuffixChoice> {
            SuffixChoice("owo"),
            SuffixChoice("owo~"),
            SuffixChoice("owo~~")
        }),
        SuffixChoice(std::vector<SuffixChoice> {
            SuffixChoice("uwu"),
            SuffixChoice("uwu~"),
            SuffixChoice("uwu~~")
        }),
        SuffixChoice("-.-"),
        SuffixChoice(">w<"),
        SuffixChoice(":3"),
        SuffixChoice(std::vector<SuffixChoice> {
            SuffixChoice("nya"),
            SuffixChoice("nya~"),
            SuffixChoice("nya~~"),
            SuffixChoice("nyaa"),
            SuffixChoice("nyaa~"),
            SuffixChoice("nyaa~~")
        }),
        SuffixChoice(std::vector<SuffixChoice> {
            SuffixChoice(">_<"),
            SuffixChoice(">-<")
        }),
        SuffixChoice(":flushed:"),
        SuffixChoice(std::vector<SuffixChoice> {
           SuffixChoice("^^"),
           SuffixChoice("^^;;")
        }),
        SuffixChoice(std::vector<SuffixChoice> {
            SuffixChoice("w"),
            SuffixChoice("ww")
        }),
        SuffixChoice(",")
    });

    typedef std::string (*ReplacementFunc)(const boost::smatch& match);
    class Replacement {
    private:
        boost::regex _regex;
        ReplacementFunc _replacement;
    public:
        Replacement(boost::regex regex, ReplacementFunc replacement) {
            _regex = regex;
            _replacement = replacement;
        }

        boost::regex& getRegex() { return _regex; }
        ReplacementFunc getReplacement() { return _replacement; }
    };

    static std::vector<Replacement> const replacements = {
        // . to !
        // match a . with a space or string end after it
        Replacement(boost::regex(R"(\.(?= |$))"), [](const boost::smatch& match) {
            return getChance(_settings.periodToExclamationChance) ? "!" : match.str();
        }),
        // duplicate characters
        Replacement(boost::regex(R"([,!])"), [](const boost::smatch& match) {
            if(!getChance(_settings.duplicateCharactersChance))
                return match.str();
            std::uniform_int_distribution<int> dist(_settings.duplicateCharactersAmount - 1, 2 * (_settings.duplicateCharactersAmount - 1));
            int amount = dist(gen);

            std::string newMatch(match.str());
            for(int i = 0; i < amount; i++)
                newMatch.append(",");
            return newMatch;
        }),
        // simple and word replacements
        // match a word
        Replacement(boost::regex(R"((?<=\b)[a-zA-Z\']+(?=\b))"), [](const boost::smatch& match) {
            std::string currentMatch = match.str();
            bool caps = isCaps(currentMatch);
            currentMatch = toLower(currentMatch);
            if(wordReplacements.contains(currentMatch))
                currentMatch = wordReplacements.at(currentMatch); // only replace whole words
            for(auto pair : simpleReplacements) {
                // don't replace whole words
                bool wholeWord = false;
                for(boost::sregex_iterator i = boost::sregex_iterator(currentMatch.begin(), currentMatch.end(), pair.first); i != boost::sregex_iterator(); ++i) {
                    boost::smatch m = *i;
                    if(m.str() != currentMatch)
                        continue;
                    wholeWord = true;
                    break;
                }
                if(wholeWord)
                    continue;
                currentMatch = boost::regex_replace(currentMatch, pair.first, pair.second);
            }
            return caps ? toUpper(currentMatch) : currentMatch;
        }),
        // stutter
        // match beginning of a word
        Replacement(boost::regex(R"((?:(?<= )|(?<=^))[a-zA-Z])"), [](const boost::smatch& match) {
            std::string mastchStr = match.str();
            return getChance(_settings.stutterChance) ? std::format("{}-{}", mastchStr, mastchStr) : mastchStr;
        }),
        // suffixes
        Replacement(boost::regex(R"((?<=[.!?,;\-])(?= )|(?=$))"), [](const boost::smatch& match) {
            std::string suffix = "";
            if(getChance(_settings.presuffixChance))
                suffix = presuffixes.choose();
            if(getChance(_settings.suffixChance)) {
                suffix += " ";
                suffix += suffixes.choose();
            }
            return suffix;
        })
    };

    void seedFrom(const void* ptr) {
        gen.seed(rd + (uintptr_t)ptr);
    }

    std::string uwuify(std::string text) {
        if(isNullOrWhiteSpace(text))
            return text;

        for(auto replacement : replacements)
            text = boost::regex_replace(text, replacement.getRegex(), replacement.getReplacement());

        return text;
    }
}
