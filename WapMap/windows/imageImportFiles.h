#pragma once
#include "imageImportData.h"
#include <filesystem>
#include <windows.h>

namespace ImageImportFiles {
struct Output { int id; ImageImportData::Bytes bytes; };
inline bool Save(const std::filesystem::path& directory, const std::vector<Output>& output, std::string& error) {
    namespace fs = std::filesystem;
    std::vector<fs::path> temporary, committed;
    const auto cleanup = [&] { std::error_code ec; for (auto& p:temporary) fs::remove(p,ec); for (auto& p:committed) fs::remove(p,ec); };
    try {
        if (output.empty()) throw std::runtime_error("No images to import.");
        fs::create_directories(directory);
        for (const auto& item:output) {
            const auto destination=directory/(std::to_string(item.id)+".pid");
            if (item.id<0 || item.bytes.empty() || fs::exists(destination)) throw std::runtime_error("An output ID already exists or is invalid.");
            const auto staged=directory/(".wapmap-import-"+std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64())+"-"+std::to_string(temporary.size())+".tmp");
            HANDLE file=CreateFileW(staged.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
            if (file==INVALID_HANDLE_VALUE) throw std::system_error(GetLastError(),std::system_category());
            temporary.push_back(staged);
            DWORD written=0;
            const bool ok=item.bytes.size()<=MAXDWORD && WriteFile(file,item.bytes.data(),static_cast<DWORD>(item.bytes.size()),&written,nullptr) && written==item.bytes.size() && FlushFileBuffers(file);
            const DWORD code=ok ? ERROR_SUCCESS : GetLastError();
            CloseHandle(file);
            if (!ok) throw std::system_error(code ? code : ERROR_WRITE_FAULT,std::system_category());
        }
        for (size_t i=0;i<output.size();++i) {
            const auto destination=directory/(std::to_string(output[i].id)+".pid");
            if (!MoveFileExW(temporary[i].c_str(),destination.c_str(),MOVEFILE_WRITE_THROUGH)) throw std::system_error(GetLastError(),std::system_category());
            committed.push_back(destination);
        }
        return true;
    } catch (const std::exception& e) { error=e.what(); cleanup(); return false; }
}
}
