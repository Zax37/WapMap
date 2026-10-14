/*
** WapMap HGE compatibility layer
** HGE-derived interface portions are covered by ../LICENSE.txt.
*/
#include "hge.h"
#include "hge_internal.h"
#include "win32.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/String.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/VideoMode.hpp>

#include <algorithm>
#include <deque>
#include <stdexcept>
#include <string>

namespace {

int MapKeyboardKey(const sf::Keyboard::Key key) {
    using Key = sf::Keyboard::Key;
    if (key >= Key::A && key <= Key::Z) {
        return HGEK_A + static_cast<int>(key) - static_cast<int>(Key::A);
    }
    if (key >= Key::Num0 && key <= Key::Num9) {
        return HGEK_0 + static_cast<int>(key) - static_cast<int>(Key::Num0);
    }
    if (key >= Key::Numpad0 && key <= Key::Numpad9) {
        return HGEK_NUMPAD0 + static_cast<int>(key) - static_cast<int>(Key::Numpad0);
    }
    if (key >= Key::F1 && key <= Key::F12) {
        return HGEK_F1 + static_cast<int>(key) - static_cast<int>(Key::F1);
    }

    switch (key) {
    case Key::Escape: return HGEK_ESCAPE;
    case Key::LControl:
    case Key::RControl: return HGEK_CTRL;
    case Key::LShift:
    case Key::RShift: return HGEK_SHIFT;
    case Key::LAlt:
    case Key::RAlt: return HGEK_ALT;
    case Key::LSystem: return HGEK_LWIN;
    case Key::RSystem: return HGEK_RWIN;
    case Key::Menu: return HGEK_APPS;
    case Key::LBracket: return HGEK_LBRACKET;
    case Key::RBracket: return HGEK_RBRACKET;
    case Key::Semicolon: return HGEK_SEMICOLON;
    case Key::Comma: return HGEK_COMMA;
    case Key::Period: return HGEK_PERIOD;
    case Key::Apostrophe: return HGEK_APOSTROPHE;
    case Key::Slash: return HGEK_SLASH;
    case Key::Backslash: return HGEK_BACKSLASH;
    case Key::Grave: return HGEK_GRAVE;
    case Key::Equal: return HGEK_EQUALS;
    case Key::Hyphen: return HGEK_MINUS;
    case Key::Space: return HGEK_SPACE;
    case Key::Enter: return HGEK_ENTER;
    case Key::Backspace: return HGEK_BACKSPACE;
    case Key::Tab: return HGEK_TAB;
    case Key::PageUp: return HGEK_PGUP;
    case Key::PageDown: return HGEK_PGDN;
    case Key::End: return HGEK_END;
    case Key::Home: return HGEK_HOME;
    case Key::Insert: return HGEK_INSERT;
    case Key::Delete: return HGEK_DELETE;
    case Key::Add: return HGEK_ADD;
    case Key::Subtract: return HGEK_SUBTRACT;
    case Key::Multiply: return HGEK_MULTIPLY;
    case Key::Divide: return HGEK_DIVIDE;
    case Key::Left: return HGEK_LEFT;
    case Key::Right: return HGEK_RIGHT;
    case Key::Up: return HGEK_UP;
    case Key::Down: return HGEK_DOWN;
    case Key::Pause: return HGEK_PAUSE;
    default: return HGEK_NO_KEY;
    }
}

int EncodeTextCharacter(const char32_t character) {
    if (character > 0x10FFFF || (character >= 0xD800 && character <= 0xDFFF)) {
        return 0;
    }

    wchar_t wide[2]{};
    int wideLength = 1;
    if (character <= 0xFFFF) {
        wide[0] = static_cast<wchar_t>(character);
    } else {
        const auto codepoint = character - 0x10000;
        wide[0] = static_cast<wchar_t>(0xD800 + (codepoint >> 10));
        wide[1] = static_cast<wchar_t>(0xDC00 + (codepoint & 0x3FF));
        wideLength = 2;
    }

    char encoded[2]{};
    BOOL usedDefault{};
    const auto count = WideCharToMultiByte(CP_ACP, 0, wide, wideLength, encoded,
                                           static_cast<int>(sizeof(encoded)), nullptr,
                                           &usedDefault);
    if (count <= 0) {
        return 0;
    }
    return static_cast<unsigned char>(encoded[0])
           | (count > 1 ? static_cast<unsigned char>(encoded[1]) << 8 : 0);
}

hgeCallback GetCallback(const std::map<hgeFuncState, hgeCallback>& callbacks,
                        const hgeFuncState state) {
    const auto it = callbacks.find(state);
    return it == callbacks.end() ? nullptr : it->second;
}

}

HGE::HGE() : impl_(std::make_unique<Impl>()) {}

HGE::~HGE() {
    System_Shutdown();
}

void HGE::Release() {}

bool HGE::System_Initiate() {
    auto& state = *impl_;
    if (state.window.isOpen()) {
        state.window.close();
    }

    const auto width = System_GetState(HGE_SCREENWIDTH);
    const auto height = System_GetState(HGE_SCREENHEIGHT);
    if (width <= 0 || height <= 0) {
        state.errorMessage = "Window dimensions must be positive.";
        return false;
    }

    const auto titleIt = state.stringStates.find(HGE_TITLE);
    const std::string title = titleIt == state.stringStates.end() ? "HGE" : titleIt->second;
    const auto style = System_GetState(HGE_WINDOWCAPTION) ? sf::Style::Default : sf::Style::None;
    state.window.create(sf::VideoMode({static_cast<unsigned>(width), static_cast<unsigned>(height)}),
                        title, style);
    if (!state.window.isOpen()) {
        state.errorMessage = "SFML could not create the application window.";
        return false;
    }
    state.window.setVerticalSyncEnabled(true);
    state.window.setView(sf::View(sf::FloatRect(
        {0.f, 0.f}, {static_cast<float>(width), static_cast<float>(height)})));

    if (const auto icon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(100))) {
        const auto handle = state.window.getNativeHandle();
        SendMessage(handle, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(icon));
        SendMessage(handle, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(icon));
    }

    if (!InstallWin32WindowFeatures(state.window.getNativeHandle(), this,
                                    System_GetState(HGE_ACCEPTFILES))) {
        state.errorMessage = "Could not install Win32 window hooks or file-drop support.";
        state.window.close();
        return false;
    }

    state.applicationClock.restart();
    state.frameClock.restart();
    state.fpsClock.restart();
    state.delta = 0.f;
    state.fps = 0;
    state.framesSinceFpsUpdate = 0;
    state.errorMessage.clear();
    state.initialized = true;
    return true;
}

void HGE::System_Shutdown() {
    for (auto& voice : impl_->audioVoices) {
        if (voice.sound) {
            voice.sound->stop();
        }
        ++voice.generation;
    }
    if (impl_->window.isOpen()) {
        RemoveWin32WindowFeatures(impl_->window.getNativeHandle());
        impl_->window.close();
    }
    impl_->initialized = false;
}

bool HGE::System_Start() {
    if (!impl_->initialized || !impl_->window.isOpen()) {
        impl_->errorMessage = "System_Initiate must succeed before System_Start.";
        return false;
    }

    while (impl_->window.isOpen()) {
        if (!RunOneFrame()) {
            break;
        }
    }
    return true;
}

char* HGE::System_GetErrorMessage() {
    return impl_->errorMessage.data();
}

void HGE::System_Log(const char*, ...) {}

std::vector<std::string>& HGE::System_GetDraggedFiles() {
    return impl_->draggedFiles;
}

void HGE::System_GetDroppedFilesPosition(int* x, int* y) {
    GetWin32DroppedFilesPosition(x, y);
}

void HGE::System_DoManualMainLoop() {
    if (impl_->initialized && impl_->window.isOpen()) {
        RunOneFrame();
    }
}

void HGE::System_SetState(const hgeBoolState state, const bool value) {
    impl_->boolStates[state] = value;
}

void HGE::System_SetState(const hgeFuncState state, const hgeCallback value) {
    impl_->callbacks[state] = value;
}

void HGE::System_SetState(hgeHwndState, HWND) {}

void HGE::System_SetState(const hgeIntState state, const int value) {
    impl_->intStates[state] = value;
    if (state == HGE_FXVOLUME) {
        const auto masterVolume = static_cast<float>(std::clamp(value, 0, 100)) / 100.f;
        for (auto& voice : impl_->audioVoices) {
            if (voice.sound) {
                voice.sound->setVolume(static_cast<float>(voice.volume) * masterVolume);
            }
        }
    }
}

void HGE::System_SetState(const hgeStringState state, const char* value) {
    if (state == HGE_ICON) {
        return;
    }
    if (value) {
        impl_->stringStates[state] = value;
        if (state == HGE_TITLE && impl_->window.isOpen()) {
            impl_->window.setTitle(sf::String(value));
        }
    } else {
        impl_->stringStates.erase(state);
    }
}

bool HGE::System_GetState(const hgeBoolState state) {
    const auto it = impl_->boolStates.find(state);
    if (it != impl_->boolStates.end()) {
        return it->second;
    }

    switch (state) {
    case HGE_WINDOWED:
    case HGE_TEXTUREFILTER:
    case HGE_USESOUND:
    case HGE_HIDEMOUSE:
    case HGE_SHOWSPLASH:
        return true;
    default:
        return false;
    }
}

hgeCallback HGE::System_GetState(const hgeFuncState state) {
    return GetCallback(impl_->callbacks, state);
}

HWND HGE::System_GetState(const hgeHwndState state) {
    if (state == HGE_HWND && impl_->window.isOpen()) {
        return impl_->window.getNativeHandle();
    }
    return nullptr;
}

int HGE::System_GetState(const hgeIntState state) {
    const auto it = impl_->intStates.find(state);
    return it == impl_->intStates.end() ? 0 : it->second;
}

const char* HGE::System_GetState(const hgeStringState state) {
    const auto it = impl_->stringStates.find(state);
    return it == impl_->stringStates.end() ? nullptr : it->second.c_str();
}

void HGE::Input_GetMousePos(float* x, float* y) {
    if (x) *x = static_cast<float>(impl_->mouseX);
    if (y) *y = static_cast<float>(impl_->mouseY);
}

void HGE::Input_SetMousePos(const float x, const float y) {
    if (impl_->window.isOpen()) {
        sf::Mouse::setPosition({static_cast<int>(x), static_cast<int>(y)}, impl_->window);
    }
}

int HGE::Input_GetMouseWheel() {
    return impl_->mouseWheel;
}

bool HGE::Input_IsMouseOver() {
    return impl_->mouseOver;
}

bool HGE::Input_KeyDown(const int key) {
    return key >= 0 && key < static_cast<int>(impl_->pressedKeys.size())
           && (impl_->pressedKeys[static_cast<size_t>(key)]
               || (impl_->currentKeys[static_cast<size_t>(key)]
                   && !impl_->previousKeys[static_cast<size_t>(key)]));
}

bool HGE::Input_KeyUp(const int key) {
    return key >= 0 && key < static_cast<int>(impl_->releasedKeys.size())
           && (impl_->releasedKeys[static_cast<size_t>(key)]
               || (!impl_->currentKeys[static_cast<size_t>(key)]
                   && impl_->previousKeys[static_cast<size_t>(key)]));
}

bool HGE::Input_GetKeyState(const int key) {
    return key >= 0 && key < static_cast<int>(impl_->currentKeys.size())
           && impl_->currentKeys[static_cast<size_t>(key)];
}

int HGE::Input_GetChar() {
    return impl_->inputChar;
}

bool HGE::Input_GetEvent(hgeInputEvent* event) {
    if (!event || impl_->inputEvents.empty()) {
        return false;
    }
    *event = impl_->inputEvents.front();
    impl_->inputEvents.pop_front();
    return true;
}

int HGE::Random_Int(const int min, const int max) {
    if (min > max) {
        throw std::invalid_argument("Random_Int requires min <= max.");
    }
    return std::uniform_int_distribution<int>(min, max)(impl_->random);
}

float HGE::Random_Float(const float min, const float max) {
    if (!(min <= max)) {
        throw std::invalid_argument("Random_Float requires min <= max.");
    }
    return std::uniform_real_distribution<float>(min, max)(impl_->random);
}

float HGE::Timer_GetTime() {
    return impl_->applicationClock.getElapsedTime().asSeconds();
}

float HGE::Timer_GetDelta() {
    return impl_->delta;
}

float HGE::Timer_GetDeltaRealtime() {
    return impl_->delta;
}

int HGE::Timer_GetFPS() {
    return impl_->fps;
}

bool HGE::RunOneFrame() {
    auto& state = *impl_;
    state.delta = state.frameClock.restart().asSeconds();
    state.previousKeys = state.currentKeys;
    state.pressedKeys.fill(false);
    state.releasedKeys.fill(false);
    state.inputEvents.clear();
    state.inputChar = 0;
    state.mouseWheel = 0;

    const auto width = std::max(1, System_GetState(HGE_SCREENWIDTH));
    const auto height = std::max(1, System_GetState(HGE_SCREENHEIGHT));
    const auto clampMousePosition = [width, height](const sf::Vector2i position) {
        return sf::Vector2i(std::clamp(position.x, 0, width - 1),
                            std::clamp(position.y, 0, height - 1));
    };
    const auto queueInputEvent = [&state](const int type, const int key, int flags,
                                          const int chr, const int wheel,
                                          const int x, const int y) {
        if (state.currentKeys[HGEK_SHIFT]) flags |= HGEINP_SHIFT;
        if (state.currentKeys[HGEK_CTRL]) flags |= HGEINP_CTRL;
        if (state.currentKeys[HGEK_ALT]) flags |= HGEINP_ALT;
        if (GetKeyState(VK_CAPITAL) & 1) flags |= HGEINP_CAPSLOCK;
        if (GetKeyState(VK_SCROLL) & 1) flags |= HGEINP_SCROLLLOCK;
        if (GetKeyState(VK_NUMLOCK) & 1) flags |= HGEINP_NUMLOCK;
        state.inputEvents.push_back({type, key, flags, chr, wheel,
                                     static_cast<float>(x), static_cast<float>(y)});
    };
    const auto updateKey = [&state, &queueInputEvent](const int key, const bool down,
                                                      const bool repeated, const int eventType,
                                                      const int x, const int y, const int chr = 0) {
        if (key <= HGEK_NO_KEY || key >= static_cast<int>(state.currentKeys.size())) {
            return;
        }
        const auto index = static_cast<size_t>(key);
        if (down) {
            if (!state.currentKeys[index]) {
                state.currentKeys[index] = true;
                state.pressedKeys[index] = !repeated;
            }
        } else if (state.currentKeys[index]) {
            state.currentKeys[index] = false;
            state.releasedKeys[index] = true;
        }
        queueInputEvent(eventType, key, repeated ? HGEINP_REPEAT : 0, chr, 0, x, y);
    };
    const auto updateModifierState = [&state](const sf::Keyboard::Key key, const bool down) {
        using Key = sf::Keyboard::Key;
        if (key == Key::LShift) state.leftShift = down;
        if (key == Key::RShift) state.rightShift = down;
        if (key == Key::LControl) state.leftControl = down;
        if (key == Key::RControl) state.rightControl = down;
        if (key == Key::LAlt) state.leftAlt = down;
        if (key == Key::RAlt) state.rightAlt = down;
        const auto updateAggregate = [&state](const int code, const bool isDown) {
            const auto index = static_cast<size_t>(code);
            if (!state.currentKeys[index] && isDown) {
                state.pressedKeys[index] = true;
            } else if (state.currentKeys[index] && !isDown) {
                state.releasedKeys[index] = true;
            }
            state.currentKeys[index] = isDown;
        };
        updateAggregate(HGEK_SHIFT, state.leftShift || state.rightShift);
        updateAggregate(HGEK_CTRL, state.leftControl || state.rightControl);
        updateAggregate(HGEK_ALT, state.leftAlt || state.rightAlt);
    };

    while (const auto event = state.window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            const auto exitCallback = GetCallback(state.callbacks, HGE_EXITFUNC);
            if (!exitCallback || exitCallback()) {
                state.window.close();
            }
        } else if (event->is<sf::Event::FocusLost>()) {
            for (size_t key = 1; key < state.currentKeys.size(); ++key) {
                if (state.currentKeys[key]) {
                    state.currentKeys[key] = false;
                    state.releasedKeys[key] = true;
                    queueInputEvent(INPUT_KEYUP, static_cast<int>(key), 0, 0, 0,
                                    state.mouseX, state.mouseY);
                }
            }
            state.leftShift = state.rightShift = false;
            state.leftControl = state.rightControl = false;
            state.leftAlt = state.rightAlt = false;
            state.mouseOver = false;
            if (const auto callback = GetCallback(state.callbacks, HGE_FOCUSLOSTFUNC)) {
                callback();
            }
        } else if (event->is<sf::Event::FocusGained>()) {
            if (const auto callback = GetCallback(state.callbacks, HGE_FOCUSGAINFUNC)) {
                callback();
            }
        } else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            const auto key = MapKeyboardKey(keyPressed->code);
            if (key == HGEK_NO_KEY) {
                continue;
            }
            const bool repeated = state.currentKeys[static_cast<size_t>(key)];
            const bool isModifier = key == HGEK_SHIFT || key == HGEK_CTRL || key == HGEK_ALT;
            if (isModifier) {
                updateModifierState(keyPressed->code, true);
            }
            if (keyPressed->alt && keyPressed->code == sf::Keyboard::Key::Enter) {
                if (const auto callback = GetCallback(state.callbacks, HGE_FULLSCREENTOGGLEFUNC)) {
                    callback();
                }
                continue;
            }
            if (keyPressed->alt && keyPressed->code == sf::Keyboard::Key::F4) {
                const auto callback = GetCallback(state.callbacks, HGE_EXITFUNC);
                if (!callback || callback()) {
                    state.window.close();
                }
                continue;
            }
            const auto chr = key == HGEK_ENTER ? '\r' : key == HGEK_BACKSPACE ? '\b' : 0;
            if (isModifier) {
                queueInputEvent(INPUT_KEYDOWN, key, repeated ? HGEINP_REPEAT : 0,
                                chr, 0, state.mouseX, state.mouseY);
            } else {
                updateKey(key, true, repeated, INPUT_KEYDOWN, state.mouseX, state.mouseY, chr);
            }
        } else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>()) {
            const auto key = MapKeyboardKey(keyReleased->code);
            if (key == HGEK_NO_KEY) {
                continue;
            }
            const bool isModifier = key == HGEK_SHIFT || key == HGEK_CTRL || key == HGEK_ALT;
            if (isModifier) {
                updateModifierState(keyReleased->code, false);
                queueInputEvent(INPUT_KEYUP, key, 0, 0, 0, state.mouseX, state.mouseY);
            } else {
                updateKey(key, false, false, INPUT_KEYUP, state.mouseX, state.mouseY);
            }
        } else if (const auto* textEntered = event->getIf<sf::Event::TextEntered>()) {
            state.inputChar = EncodeTextCharacter(textEntered->unicode);
            if (state.inputChar != 0) {
                for (auto it = state.inputEvents.rbegin(); it != state.inputEvents.rend(); ++it) {
                    if (it->type == INPUT_KEYDOWN && it->chr == 0) {
                        it->chr = state.inputChar;
                        break;
                    }
                }
            }
        } else if (const auto* mouseMoved = event->getIf<sf::Event::MouseMoved>()) {
            const auto position = clampMousePosition(mouseMoved->position);
            state.mouseX = position.x;
            state.mouseY = position.y;
            queueInputEvent(INPUT_MOUSEMOVE, 0, 0, 0, 0, position.x, position.y);
        } else if (event->is<sf::Event::MouseEntered>()) {
            state.mouseOver = true;
        } else if (event->is<sf::Event::MouseLeft>()) {
            state.mouseOver = false;
        } else if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
            const auto position = clampMousePosition(mousePressed->position);
            int key = HGEK_NO_KEY;
            switch (mousePressed->button) {
            case sf::Mouse::Button::Left: key = HGEK_LBUTTON; break;
            case sf::Mouse::Button::Right: key = HGEK_RBUTTON; break;
            case sf::Mouse::Button::Middle: key = HGEK_MBUTTON; break;
            default: break;
            }
            if (key != HGEK_NO_KEY) {
                updateKey(key, true, state.currentKeys[static_cast<size_t>(key)],
                          INPUT_MBUTTONDOWN, position.x, position.y);
            }
        } else if (const auto* mouseReleased = event->getIf<sf::Event::MouseButtonReleased>()) {
            const auto position = clampMousePosition(mouseReleased->position);
            int key = HGEK_NO_KEY;
            switch (mouseReleased->button) {
            case sf::Mouse::Button::Left: key = HGEK_LBUTTON; break;
            case sf::Mouse::Button::Right: key = HGEK_RBUTTON; break;
            case sf::Mouse::Button::Middle: key = HGEK_MBUTTON; break;
            default: break;
            }
            if (key != HGEK_NO_KEY) {
                updateKey(key, false, false, INPUT_MBUTTONUP, position.x, position.y);
            }
        } else if (const auto* wheelScrolled = event->getIf<sf::Event::MouseWheelScrolled>()) {
            if (wheelScrolled->wheel == sf::Mouse::Wheel::Vertical) {
                const auto position = clampMousePosition(wheelScrolled->position);
                state.mouseX = position.x;
                state.mouseY = position.y;
                state.wheelRemainder += wheelScrolled->delta;
                const auto wheel = static_cast<int>(state.wheelRemainder);
                state.wheelRemainder -= static_cast<float>(wheel);
                state.mouseWheel += wheel;
                if (wheel != 0) {
                    queueInputEvent(INPUT_MOUSEWHEEL, 0, 0, 0, wheel,
                                    position.x, position.y);
                }
            }
        } else if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            state.intStates[HGE_SCREENWIDTH] = static_cast<int>(resized->size.x);
            state.intStates[HGE_SCREENHEIGHT] = static_cast<int>(resized->size.y);
            state.window.setView(sf::View(sf::FloatRect(
                {0.f, 0.f},
                {static_cast<float>(resized->size.x), static_cast<float>(resized->size.y)})));
            if (const auto callback = GetCallback(state.callbacks, HGE_RESIZEFUNC)) {
                callback();
            }
        }
    }

    if (!state.window.isOpen()) {
        return false;
    }
    if (const auto frameCallback = GetCallback(state.callbacks, HGE_FRAMEFUNC);
        frameCallback && frameCallback()) {
        state.window.close();
        return false;
    }
    if (const auto renderCallback = GetCallback(state.callbacks, HGE_RENDERFUNC)) {
        renderCallback();
    }
    state.window.display();

    ++state.framesSinceFpsUpdate;
    const auto fpsElapsed = state.fpsClock.getElapsedTime().asSeconds();
    if (fpsElapsed >= 1.f) {
        state.fps = static_cast<int>(state.framesSinceFpsUpdate / fpsElapsed);
        state.framesSinceFpsUpdate = 0;
        state.fpsClock.restart();
    }
    return true;
}

extern "C" HGE* HGE_CALL hgeCreate(const int version) {
    if (version != HGE_VERSION) {
        return nullptr;
    }
    static HGE instance;
    return &instance;
}
