#include "i18n.hpp"

namespace core {
    std::unordered_map<std::string, std::string> I18n::strings;

    void I18n::Set(const std::string& key, const std::string& value) {
        strings[key] = value;
    }

    const char* I18n::Get(const std::string& key) {
        auto it = strings.find(key);
        if (it != strings.end()) {
            return it->second.c_str();
        }
        return key.c_str(); // Fallback to key
    }

    void I18n::LoadLanguage(const std::string& lang_code) {
        strings.clear();
        // Mock loading
        if (lang_code == "ja") {
            Set("File", "ファイル");
            Set("Edit", "編集");
            Set("Help", "ヘルプ");
            Set("Welcome", "ようこそ");
            Set("Cancel", "キャンセル");
        }
        // en is default (keys)
    }
}
