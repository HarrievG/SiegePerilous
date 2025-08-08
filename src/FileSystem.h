#ifndef MODERN_FILESYSTEM_H
#define MODERN_FILESYSTEM_H

#include <chrono>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <glaze/glaze.hpp>

// Use a namespace alias for convenience
namespace fs = std::filesystem;

// A constant for a non-existent file's timestamp
const auto FILE_NOT_FOUND_TIMESTAMP = fs::file_time_type::min();

struct FileSystemConfig {
	std::string basePath{};				// The root directory where game data is stored.
	std::string savePath{};				// The directory for saves, configs, and other user files.
	std::string mainGameName{};			// The primary game/mod directory (e.g., "d3xp", "mymod").
	std::string baseGameName{};			// An optional secondary game directory to search.

	struct glaze {
		using T = FileSystemConfig;
		static constexpr auto value = glz::object(
			"basePath", &T::basePath, "Root path for game assets (e.g., 'C:/Doom3')",
			"savePath", &T::savePath, "Path for user-generated files (saves, configs)",
			"mainGameName", &T::mainGameName, "The primary game/mod directory to search first",
			"baseGameName", &T::baseGameName, "A base game directory to search as a fallback"
		);
	};
};

// Scoped enum for file opening modes
enum class FileMode {
    Read,
    Write,
    Append
};

// A structure to hold the results of a directory listing
struct FileList {
    fs::path basePath;
    std::vector<fs::path> files;
};

// Abstract base class for the FileSystem interface
class FileSystem {
public:
    virtual ~FileSystem() = default;

    // Initializes the file system with given base and save paths.
    // The search order priority is: mainGame > baseGame > "base"
    virtual void Init(const fs::path& base_path, const fs::path& save_path,
                      const std::string& main_game_name = "",
                      const std::string& base_game_name = "") = 0;

    // Shuts down the file system and clears all paths.
    virtual void Shutdown() = 0;

    // Returns true if the file system is initialized.
    virtual bool IsInitialized() const = 0;

    // Lists files with a given extension in a directory across all search paths.
    // The extension must include the dot (e.g., ".txt"). Use ".*" for all files.
    // Use "/" as the extension to list only directories.
    virtual std::unique_ptr<FileList> ListFiles(
        const fs::path& relativePath, const std::string& extension,
        bool sort = false, bool fullRelativePath = false) = 0;

    // Recursively lists files with a given extension across all search paths.
    virtual std::unique_ptr<FileList> ListFilesTree(
        const fs::path& relativePath, const std::string& extension,
        bool sort = false) = 0;

    // Converts a relative path to a full OS-specific path based on a root
    // (e.g., the save path or base path).
    virtual fs::path RelativeToOSPath(const fs::path& relativePath,
                                      const std::string basePathName = "" ) const = 0;

    // Reads a complete file into a vector of bytes.
    // Returns an empty optional on failure.
    virtual std::optional<std::vector<char>> ReadFile(
        const fs::path& relativePath) = 0;

    // Gets the last modification time of a file.
    virtual fs::file_time_type GetFileTimestamp(
        const fs::path& relativePath) = 0;

    // Writes a buffer to a file in the specified path (save or base).
    // Returns the number of bytes written, or a negative value on failure.
    virtual int WriteFile(const fs::path& relativePath,
                          const std::vector<char>& buffer,
                          const std::string& basePathName) = 0;

    // Removes a file. Primarily acts on the save path.
    virtual void RemoveFile(const fs::path& relativePath) = 0;

    // Renames a file within a specific path (save or base).
    virtual bool RenameFile(const fs::path& oldRelativePath,
                            const fs::path& newRelativePath,
                            const std::string& basePathName) = 0;

    // Compares two paths for equality, ignoring case and separator style.
    virtual bool FilenameCompare(const fs::path& p1, const fs::path& p2) const = 0;

    // Returns the length of a file in bytes, or -1 if it doesn't exist.
    virtual long long GetFileLength(const fs::path& relativePath) = 0;

};

extern FileSystem* fileSystem;

#endif // MODERN_FILESYSTEM_H