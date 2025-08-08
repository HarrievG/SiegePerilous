#include "FileSystem.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <system_error>
#include <unordered_set>

// Helper struct to define a single search location
struct SearchPath {
    fs::path path;     // The root path (e.g., C:\Doom3)
    fs::path gamedir;  // The game directory (e.g., "base", "d3xp")

    bool operator==(const SearchPath& other) const {
        return path == other.path && gamedir == other.gamedir;
    }
};

// Define a hash function for SearchPath to use it in std::unordered_set
namespace std {
template <>
struct hash<SearchPath> {
    size_t operator()(const SearchPath& sp) const {
        // Combine hashes of the two path members
        return hash<fs::path>()(sp.path) ^
               (hash<fs::path>()(sp.gamedir) << 1);
    }
};
} // namespace std

class ModernFileSystem final : public FileSystem {
public:
    ModernFileSystem() : initialized(false) {}

    void Init(const fs::path& base_path, const fs::path& save_path,
              const std::string& main_game_name,
              const std::string& base_game_name) override;
    void Shutdown() override;
    bool IsInitialized() const override { return initialized; }

    std::unique_ptr<FileList> ListFiles(
        const fs::path& relativePath, const std::string& extension,
        bool sort, bool fullRelativePath) override;
    std::unique_ptr<FileList> ListFilesTree(
        const fs::path& relativePath, const std::string& extension,
        bool sort) override;

    fs::path RelativeToOSPath(const fs::path& relativePath,
                              const std::string basePathName) const override;

    std::optional<std::vector<char>> ReadFile(
        const fs::path& relativePath) override;
    fs::file_time_type GetFileTimestamp(const fs::path& relativePath) override;

    int WriteFile(const fs::path& relativePath,
                  const std::vector<char>& buffer,
                  const std::string& basePathName) override;
    void RemoveFile(const fs::path& relativePath) override;
    bool RenameFile(const fs::path& oldRelativePath,
                    const fs::path& newRelativePath,
                    const std::string& basePathName) override;

    bool FilenameCompare(const fs::path& p1, const fs::path& p2) const override;
    long long GetFileLength(const fs::path& relativePath) override;

private:
    std::optional<fs::path> FindFile(const fs::path& relativePath) const;
    void AddGameDirectory(const fs::path& path, const fs::path& dir);
    void SetupGameDirectories(const std::string& gameName);

    bool initialized;
    fs::path rootBasePath;
    fs::path rootSavePath;
    std::string mainGameName;
    
    // Search paths are iterated in reverse, so the last one added has the highest priority.
    std::vector<SearchPath> searchPaths;
};

// Factory function implementation
FileSystem * CreateFileSystem()
{
    //return std::make_unique<ModernFileSystem>();
	return new ModernFileSystem();
}

void ModernFileSystem::Init(const fs::path& base_path,
                            const fs::path& save_path,
                            const std::string& main_game_name,
                            const std::string& base_game_name) {
    if (initialized) {
        Shutdown();
    }
    std::cout << "------ Initializing File System ------" << std::endl;

    rootBasePath = fs::absolute(base_path);
    rootSavePath = fs::absolute(save_path);
    mainGameName = main_game_name;

    if (!fs::exists(rootBasePath)) {
        throw std::runtime_error("Base path does not exist: " +
                                 rootBasePath.string());
    }
    if (!fs::exists(rootSavePath)) {
        std::cout << "Save path does not exist, creating: "
                  << rootSavePath.string() << std::endl;
        fs::create_directories(rootSavePath);
    }

    // Setup search paths in order of priority (last added is checked first)
    // 1. Default game ("base")
    SetupGameDirectories("base");

    // 2. Base game override (e.g., "d3xp")
    if (!base_game_name.empty() && base_game_name != "base") {
        SetupGameDirectories(base_game_name);
    }

    // 3. Main mod/game (e.g., "mymod")
    if (!main_game_name.empty() && main_game_name != "base" &&
        main_game_name != base_game_name) {
        SetupGameDirectories(main_game_name);
    }

    initialized = true;
    std::cout << "File system initialized." << std::endl;
    std::cout << "  Base Path: " << rootBasePath.string() << std::endl;
    std::cout << "  Save Path: " << rootSavePath.string() << std::endl;
    std::cout << "  Search Paths (Priority High to Low):" << std::endl;
    for (auto it = searchPaths.rbegin(); it != searchPaths.rend(); ++it) {
        std::cout << "    - " << (it->path / it->gamedir).string() << std::endl;
    }
    std::cout << "--------------------------------------" << std::endl;
}

void ModernFileSystem::Shutdown() {
    searchPaths.clear();
    initialized = false;
    std::cout << "File system shut down." << std::endl;
}

void ModernFileSystem::AddGameDirectory(const fs::path& path,
                                        const fs::path& dir) {
    SearchPath sp = {path, dir};
    // Avoid adding duplicate search paths
    if (std::find(searchPaths.begin(), searchPaths.end(), sp) ==
        searchPaths.end()) {
        searchPaths.push_back(sp);
    }
}

void ModernFileSystem::SetupGameDirectories(const std::string& gameName) {
    // The original engine searched both save and base paths for reading.
    // The save path is also the primary write location.
    // We add the base path first, so it has lower read priority.
    if (!rootBasePath.empty()) {
        AddGameDirectory(rootBasePath, gameName);
    }
    if (!rootSavePath.empty()) {
        AddGameDirectory(rootSavePath, gameName);
    }
}

std::optional<fs::path> ModernFileSystem::FindFile(  const fs::path& relativePath) const {

    if (!initialized || relativePath.empty() || relativePath.is_absolute() ||
        relativePath.string().find("..") != std::string::npos) {
        return std::nullopt;
    }

    // Iterate paths from last to first for highest priority
    for (auto it = searchPaths.rbegin(); it != searchPaths.rend(); ++it) {
        fs::path fullPath = it->path / it->gamedir / relativePath;
        if (fs::exists(fullPath) && fs::is_regular_file(fullPath)) {
            return fullPath;
        }
    }

    return std::nullopt;
}

long long ModernFileSystem::GetFileLength(const fs::path& relativePath) {
    if (auto fullPathOpt = FindFile(relativePath)) {
        std::error_code ec;
        long long size = fs::file_size(*fullPathOpt, ec);
        return ec ? -1 : size;
    }
    return -1;
}

fs::file_time_type ModernFileSystem::GetFileTimestamp(
    const fs::path& relativePath) {
    if (auto fullPathOpt = FindFile(relativePath)) {
        std::error_code ec;
        auto time = fs::last_write_time(*fullPathOpt, ec);
        return ec ? FILE_NOT_FOUND_TIMESTAMP : time;
    }
    return FILE_NOT_FOUND_TIMESTAMP;
}

std::optional<std::vector<char>> ModernFileSystem::ReadFile(
    const fs::path& relativePath) {
    auto fullPathOpt = FindFile(relativePath);
    if (!fullPathOpt) {
        return std::nullopt;
    }

    std::ifstream file(fullPathOpt.value(), std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return std::nullopt;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (file.read(buffer.data(), size)) {
        // Ensure null-termination for text files, as in original ReadFile
        buffer.push_back('\0');
        return buffer;
    }

    return std::nullopt;
}

int ModernFileSystem::WriteFile(const fs::path& relativePath,
                              const std::vector<char>& buffer,
                              const std::string& basePathName) {
    if (!initialized || relativePath.empty() || relativePath.is_absolute()) {
        return -1;
    }

    fs::path write_path =
        (basePathName == "fs_savepath") ? rootSavePath : rootBasePath;

    // Default to the highest-priority game name if set, otherwise "base"
    std::string gameDir = !mainGameName.empty() ? mainGameName : "base";
    fs::path fullPath = write_path / gameDir / relativePath;

    try {
        if (fullPath.has_parent_path()) {
            fs::create_directories(fullPath.parent_path());
        }

        std::ofstream file(fullPath, std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            std::cerr << "Error: Failed to open for write: " << fullPath
                      << std::endl;
            return -1;
        }

        file.write(buffer.data(), buffer.size());
        return file.good() ? static_cast<int>(buffer.size()) : -1;

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error writing file: " << e.what() << std::endl;
        return -1;
    }
}

void ModernFileSystem::RemoveFile(const fs::path& relativePath) {
    if (!initialized || relativePath.empty()) return;

    // Per original logic, try removing from both save and base paths.
    // This is generally unsafe, but we replicate it.
    std::string gameDir = !mainGameName.empty() ? mainGameName : "base";
    std::error_code ec;
    
    fs::remove(rootSavePath / gameDir / relativePath, ec);
    fs::remove(rootBasePath / gameDir / relativePath, ec);
}

bool ModernFileSystem::RenameFile(const fs::path& oldRelativePath,
                                const fs::path& newRelativePath,
                                const std::string& basePathName) {
    if (!initialized || oldRelativePath.empty() || newRelativePath.empty()) {
        return false;
    }

    fs::path write_path =
        (basePathName == "fs_savepath") ? rootSavePath : rootBasePath;
    std::string gameDir = !mainGameName.empty() ? mainGameName : "base";

    fs::path oldFullPath = write_path / gameDir / oldRelativePath;
    fs::path newFullPath = write_path / gameDir / newRelativePath;

    std::error_code ec;
    fs::rename(oldFullPath, newFullPath, ec);
    if (ec) {
        std::cerr << "Error renaming file: " << ec.message() << std::endl;
        return false;
    }
    return true;
}

fs::path ModernFileSystem::RelativeToOSPath(
    const fs::path& relativePath, const std::string basePathName = "" ) const {
    if (!initialized || relativePath.is_absolute()) {
        return relativePath;
    }
    fs::path rootPath =
        (basePathName == "fs_savepath") ? rootSavePath : rootBasePath;
    std::string gameDir = !mainGameName.empty() ? mainGameName : "base";

    return rootPath / gameDir / relativePath;
}

std::unique_ptr<FileList> ModernFileSystem::ListFiles(
    const fs::path& relativePath, const std::string& extension, bool sort,
    bool fullRelativePath) {
    if (!initialized) return nullptr;

    auto fileList = std::make_unique<FileList>();
    fileList->basePath = relativePath;
    std::unordered_set<fs::path> foundFiles;

    for (auto it = searchPaths.rbegin(); it != searchPaths.rend(); ++it) {
        fs::path dir_to_scan = it->path / it->gamedir / relativePath;
        if (!fs::exists(dir_to_scan) || !fs::is_directory(dir_to_scan)) {
            continue;
        }

        for (const auto& entry : fs::directory_iterator(dir_to_scan)) {
            bool match = false;
            if (extension == "/" && entry.is_directory()) {
                match = true;
            } else if (entry.is_regular_file()) {
                if (extension.empty() || extension == ".*" ||
                    entry.path().extension() == extension) {
                    match = true;
                }
            }

            if (match) {
                fs::path p = fullRelativePath
                                 ? (relativePath / entry.path().filename())
                                 : entry.path().filename();
                foundFiles.insert(p.lexically_normal());
            }
        }
    }

    fileList->files.assign(foundFiles.begin(), foundFiles.end());
    if (sort) {
        std::sort(fileList->files.begin(), fileList->files.end());
    }
    return fileList;
}

std::unique_ptr<FileList> ModernFileSystem::ListFilesTree(
    const fs::path& relativePath, const std::string& extension, bool sort) {
    if (!initialized) return nullptr;

    auto fileList = std::make_unique<FileList>();
    fileList->basePath = relativePath;
    std::unordered_set<fs::path> foundFiles;

    for (auto it = searchPaths.rbegin(); it != searchPaths.rend(); ++it) {
        fs::path dir_to_scan = it->path / it->gamedir / relativePath;
        if (!fs::exists(dir_to_scan) || !fs::is_directory(dir_to_scan)) {
            continue;
        }

        for (const auto& entry : fs::recursive_directory_iterator(dir_to_scan)) {
            if (entry.is_regular_file() &&
                (extension.empty() || extension == ".*" ||
                 entry.path().extension() == extension)) {
                // Create a path relative to the starting search directory
                fs::path p = fs::relative(entry.path(), it->path / it->gamedir);
                foundFiles.insert(p.lexically_normal());
            }
        }
    }

    fileList->files.assign(foundFiles.begin(), foundFiles.end());
    if (sort) {
        std::sort(fileList->files.begin(), fileList->files.end());
    }
    return fileList;
}

bool ModernFileSystem::FilenameCompare(const fs::path& p1,
                                     const fs::path& p2) const {
    const std::string s1 = p1.string();
    const std::string s2 = p2.string();

    return std::equal(s1.begin(), s1.end(), s2.begin(), s2.end(),
                      [](char a, char b) {
                          if (a == '\\' || a == '/') a = '/';
                          if (b == '\\' || b == '/') b = '/';
                          return std::tolower(a) == std::tolower(b);
                      });
}


ModernFileSystem	fileSystemLocal;
FileSystem * fileSystem = &fileSystemLocal;