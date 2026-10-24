#pragma once
#ifdef WAPMAP_ENABLE_IMGUI
#include <imgui.h>
#include <string>
namespace MapTextInput {
inline bool Multiline(const char* id, std::string& text, ImVec2 size) {
    const auto resize = [](ImGuiInputTextCallbackData* data) -> int {
        if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
            auto& value = *static_cast<std::string*>(data->UserData);
            value.resize(static_cast<size_t>(data->BufTextLen));
            data->Buf = value.data();
        }
        return 0;
    };
    return ImGui::InputTextMultiline(id, text.data(), text.capacity() + 1, size,
        ImGuiInputTextFlags_CallbackResize, resize, &text);
}
}
#endif
