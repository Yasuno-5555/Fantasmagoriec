#pragma once
#include <string>
#include <unordered_map>

namespace core {
    class I18n {
    public:
        static void Set(const std::string& key, const std::string& value);
        static const char* Get(const std::string& key);
        static void LoadLanguage(const std::string& lang_code);
    private:
        static std::unordered_map<std::string, std::string> strings;
    };
}
