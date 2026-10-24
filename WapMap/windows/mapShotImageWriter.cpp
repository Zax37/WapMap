#include "mapShotImageWriter.h"
#include <Windows.h>
#include <zlib.h>
#include <algorithm>
#include <cstdlib>
#include <limits>
#include <cctype>
#include <vector>
#include <climits>
#include <cstdint>

namespace {
unsigned char* CompressPng(unsigned char* data, int length, int* outputLength, int level) {
    uLongf size = compressBound(static_cast<uLong>(length));
    auto* result = static_cast<unsigned char*>(std::malloc(size));
    if (!result) return nullptr;
    if (compress2(result, &size, data, static_cast<uLong>(length), std::clamp(level, -1, 9)) != Z_OK || size > INT_MAX) {
        std::free(result); return nullptr;
    }
    *outputLength = static_cast<int>(size);
    return result;
}
}
#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO
#define STBIW_ZLIB_COMPRESS CompressPng
#include "../third_party/stb_image_write.h"

int MapShotImageWriter::JpegQuality(int compression) {
    if (compression == 0) return 75;
    if (compression <= 7) return 100;
    return std::clamp(100 - compression, 1, 100);
}

bool MapShotImageWriter::Save(const sf::Image& image, const std::filesystem::path& path, int option) {
    auto extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (extension != ".png" && extension != ".jpg" && extension != ".jpeg") return false;
    const auto size = image.getSize();
    if (!size.x || !size.y || (std::uint64_t(size.x) * 4 + 1) * size.y > INT_MAX) return false;
    struct Output { std::vector<unsigned char> bytes; bool failed{}; } output;
    const auto write = [](void* context, void* data, int length) {
        auto& destination = *static_cast<Output*>(context);
        if (destination.failed) return;
        try {
            const auto* begin = static_cast<unsigned char*>(data);
            destination.bytes.insert(destination.bytes.end(), begin, begin + length);
        } catch (...) { destination.failed = true; }
    };
    int encoded;
    if (extension == ".png") {
        stbi_write_png_compression_level = std::clamp(option, -1, 9);
        encoded = stbi_write_png_to_func(write, &output, static_cast<int>(size.x), static_cast<int>(size.y), 4, image.getPixelsPtr(), static_cast<int>(size.x * 4));
    } else {
        encoded = stbi_write_jpg_to_func(write, &output, static_cast<int>(size.x), static_cast<int>(size.y), 4, image.getPixelsPtr(), std::clamp(option, 1, 100));
    }
    if (!encoded || output.failed || output.bytes.empty()) return false;
    // Encode before touching the destination; commit only a completely written
    // sibling file, preserving an existing image if writing/renaming fails.
    std::filesystem::path temporary;
    HANDLE file = INVALID_HANDLE_VALUE;
    for (int attempt = 0; attempt < 20 && file == INVALID_HANDLE_VALUE; ++attempt) {
        temporary = path;
        temporary += L".wapmap-" + std::to_wstring(GetCurrentProcessId()) + L"-" + std::to_wstring(GetTickCount64()) + L"-" + std::to_wstring(attempt) + L".tmp";
        file = CreateFileW(temporary.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE && GetLastError() != ERROR_FILE_EXISTS) return false;
    }
    if (file == INVALID_HANDLE_VALUE) return false;
    bool success = true;
    size_t offset = 0;
    while (offset < output.bytes.size()) {
        DWORD written = 0;
        const DWORD count = static_cast<DWORD>(std::min(output.bytes.size() - offset, size_t(1) << 30));
        if (!WriteFile(file, output.bytes.data() + offset, count, &written, nullptr) || written != count) { success = false; break; }
        offset += written;
    }
    if (success) success = FlushFileBuffers(file);
    if (!CloseHandle(file)) success = false;
    if (success) success = MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
    if (!success) DeleteFileW(temporary.c_str());
    return success;
}
