#ifndef WAPMAP_HGE_INTERNAL_H
#define WAPMAP_HGE_INTERNAL_H

#include "hge.h"

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <array>
#include <cstdint>
#include <deque>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>

struct TextureEntry {
    std::unique_ptr<sf::Texture> ownedTexture;
    const sf::Texture* texture{};
    std::vector<DWORD> lockedPixels;
    int lockLeft{};
    int lockTop{};
    int lockWidth{};
    int lockHeight{};
    bool locked{};
    bool lockReadOnly{};
    bool pixelsCached{};
};

struct TargetEntry;

struct SurfaceEntry {
    TargetEntry* target{};
    std::vector<DWORD> lockedPixels;
};

struct TargetEntry {
    sf::RenderTexture renderTexture;
    TextureEntry texture;
    SurfaceEntry surface;
};

struct EffectEntry {
    std::shared_ptr<sf::SoundBuffer> buffer;
    bool valid{true};
};

struct AudioVoice {
    std::shared_ptr<sf::SoundBuffer> buffer;
    std::unique_ptr<sf::Sound> sound;
    std::uint64_t generation{};
    int volume{100};
};

struct ChannelEntry {
    std::size_t voice{};
    std::uint64_t generation{};
};

struct HGE::Impl {
    sf::RenderWindow window;
    sf::Clock applicationClock;
    sf::Clock frameClock;
    sf::Clock fpsClock;
    std::map<hgeBoolState, bool> boolStates;
    std::map<hgeFuncState, hgeCallback> callbacks;
    std::map<hgeIntState, int> intStates{
        {HGE_SCREENWIDTH, 800},
        {HGE_SCREENHEIGHT, 600},
        {HGE_SCREENBPP, 32},
        {HGE_SAMPLERATE, 44100},
        {HGE_FXVOLUME, 100},
        {HGE_MUSVOLUME, 100},
        {HGE_STREAMVOLUME, 100},
        {HGE_FPS, HGEFPS_UNLIMITED},
        {HGE_BORDER_WIDTH, 5}
    };
    std::map<hgeStringState, std::string> stringStates;
    std::vector<std::string> draggedFiles;
    std::string errorMessage;
    std::mt19937 random{std::random_device{}()};
    std::array<bool, 256> currentKeys{};
    std::array<bool, 256> previousKeys{};
    std::array<bool, 256> pressedKeys{};
    std::array<bool, 256> releasedKeys{};
    std::unordered_set<int> imguiCapturedKeyCodes;
    std::array<bool, 3> imguiCapturedMouseButtons{};
    bool leftShift{};
    bool rightShift{};
    bool leftControl{};
    bool rightControl{};
    bool leftAlt{};
    bool rightAlt{};
    std::deque<hgeInputEvent> inputEvents;
    int inputChar{};
    int mouseX{};
    int mouseY{};
    int physicalMouseX{};
    int physicalMouseY{};
    int physicalMouseScreenX{};
    int physicalMouseScreenY{};
    int mouseWheel{};
    float wheelRemainder{};
    bool mouseOver{};
    bool imguiInitialized{};
    bool imguiKeyboardCaptureOverride{};
    bool applicationFocused{};
    float delta{};
    int fps{};
    unsigned framesSinceFpsUpdate{};
    bool initialized{};
    std::vector<std::unique_ptr<EffectEntry>> effects;
    std::unordered_set<HEFFECT> knownEffects;
    std::vector<AudioVoice> audioVoices;
    std::vector<std::unique_ptr<ChannelEntry>> channelHandles;
    std::unordered_set<HCHANNEL> knownChannels;
    bool sceneOpen{};
    HTARGET currentTarget{};
    HTEXTURE batchTexture{};
    int batchBlend{BLEND_DEFAULT};
    sf::PrimitiveType batchPrimitive{sf::PrimitiveType::Triangles};
    std::vector<sf::Vertex> batchVertices;
    sf::Transform transform;
    int clipX{};
    int clipY{};
    int clipWidth{};
    int clipHeight{};
};

#endif
