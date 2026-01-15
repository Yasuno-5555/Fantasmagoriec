#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>

namespace fanta {
namespace utils {

    struct FileEntry {
        std::string name;
        std::string path;
        bool is_directory;
        uintmax_t size;
    };

    class FileSystemCache {
    public:
        using EntryList = std::vector<FileEntry>;

        // Refresh cache for a given path
        static EntryList ScanDirectory(const std::string& path) {
            EntryList entries;
            try {
                if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
                    return entries;
                }

                for (const auto& entry : std::filesystem::directory_iterator(path)) {
                    FileEntry e;
                    e.name = entry.path().filename().string();
                    e.path = entry.path().string();
                    e.is_directory = entry.is_directory();
                    e.size = e.is_directory ? 0 : entry.file_size();
                    entries.push_back(e);
                }

                // Sort: Directories first, then files. Alphabetical.
                std::sort(entries.begin(), entries.end(), [](const FileEntry& a, const FileEntry& b) {
                    if (a.is_directory != b.is_directory) {
                        return a.is_directory > b.is_directory; // Dirs first
                    }
                    return a.name < b.name;
                });

            } catch (...) {
                // Ignore permission errors etc for now
            }
            return entries;
        }
    };

}
}
