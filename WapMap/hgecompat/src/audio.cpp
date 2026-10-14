#include "hge_internal.h"

#include <SFML/System/Time.hpp>

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <memory>

namespace {

template <typename State>
AudioVoice* GetVoice(State& state, HCHANNEL channel) {
    if (!channel || state.knownChannels.find(channel) == state.knownChannels.end()
        || channel->voice >= state.audioVoices.size()) {
        return nullptr;
    }
    auto& voice = state.audioVoices[channel->voice];
    return voice.generation == channel->generation && voice.sound ? &voice : nullptr;
}

template <typename State>
void UpdateVoiceVolume(State& state, AudioVoice& voice) {
    const auto masterVolume = std::clamp(state.intStates[HGE_FXVOLUME], 0, 100);
    voice.sound->setVolume(static_cast<float>(voice.volume * masterVolume) / 100.f);
}

template <typename State>
HCHANNEL PlaySound(State& state, EffectEntry* effect, const int volume, const int pan,
                   const float pitch, const bool loop) {
    if (!effect || !effect->valid || !effect->buffer) {
        state.errorMessage = "Cannot play an invalid or unloaded sound effect.";
        return nullptr;
    }
    const auto soundSetting = state.boolStates.find(HGE_USESOUND);
    if (soundSetting != state.boolStates.end() && !soundSetting->second) {
        return nullptr;
    }
    if (!(pitch > 0.f)) {
        state.errorMessage = "Sound pitch must be greater than zero.";
        return nullptr;
    }

    std::size_t voiceIndex = state.audioVoices.size();
    for (std::size_t index = 0; index < state.audioVoices.size(); ++index) {
        const auto& voice = state.audioVoices[index];
        if (voice.sound->getStatus() == sf::SoundSource::Status::Stopped) {
            voiceIndex = index;
            break;
        }
    }
    if (voiceIndex == state.audioVoices.size()) {
        constexpr std::size_t maxVoices = 256;
        if (state.audioVoices.size() >= maxVoices) {
            state.errorMessage = "The sound channel limit has been reached.";
            return nullptr;
        }
        state.audioVoices.emplace_back();
    }

    auto& voice = state.audioVoices[voiceIndex];
    if (voice.sound) {
        voice.sound->stop();
        voice.sound->setBuffer(*effect->buffer);
    } else {
        voice.sound = std::make_unique<sf::Sound>(*effect->buffer);
    }
    voice.buffer = effect->buffer;
    ++voice.generation;
    voice.volume = std::clamp(volume, 0, 100);
    voice.sound->setPan(static_cast<float>(std::clamp(pan, -100, 100)) / 100.f);
    voice.sound->setPitch(pitch);
    voice.sound->setLooping(loop);
    UpdateVoiceVolume(state, voice);

    auto handle = std::make_unique<ChannelEntry>();
    handle->voice = voiceIndex;
    handle->generation = voice.generation;
    auto* result = handle.get();
    state.channelHandles.push_back(std::move(handle));
    state.knownChannels.insert(result);
    voice.sound->play();
    return result;
}

}

HEFFECT HGE::Effect_Load(const char* data, const DWORD size) {
    if (!data) {
        impl_->errorMessage = "Sound source cannot be null.";
        return nullptr;
    }
    auto buffer = std::make_shared<sf::SoundBuffer>();
    const bool loaded = size != 0
                            ? buffer->loadFromMemory(data, static_cast<std::size_t>(size))
                            : buffer->loadFromFile(std::filesystem::path(data));
    if (!loaded) {
        impl_->errorMessage = "SFML could not decode the sound effect.";
        return nullptr;
    }

    auto effect = std::make_unique<EffectEntry>();
    effect->buffer = std::move(buffer);
    auto* result = effect.get();
    impl_->effects.push_back(std::move(effect));
    impl_->knownEffects.insert(result);
    return result;
}

void HGE::Effect_Free(HEFFECT effect) {
    if (!effect || impl_->knownEffects.find(effect) == impl_->knownEffects.end()
        || !effect->valid) {
        return;
    }
    effect->valid = false;
    effect->buffer.reset();
}

HCHANNEL HGE::Effect_Play(HEFFECT effect) {
    return Effect_PlayEx(effect);
}

HCHANNEL HGE::Effect_PlayEx(HEFFECT effect, const int volume, const int pan,
                            const float pitch, const bool loop) {
    return PlaySound(*impl_, effect, volume, pan, pitch, loop);
}

void HGE::Channel_SetVolume(HCHANNEL channel, const int volume) {
    if (auto* voice = GetVoice(*impl_, channel)) {
        voice->volume = std::clamp(volume, 0, 100);
        UpdateVoiceVolume(*impl_, *voice);
    }
}

void HGE::Channel_Pause(HCHANNEL channel) {
    if (auto* voice = GetVoice(*impl_, channel)) {
        voice->sound->pause();
    }
}

void HGE::Channel_Resume(HCHANNEL channel) {
    if (auto* voice = GetVoice(*impl_, channel)
        ; voice && voice->sound->getStatus() != sf::SoundSource::Status::Playing) {
        voice->sound->play();
    }
}

void HGE::Channel_Stop(HCHANNEL channel) {
    if (auto* voice = GetVoice(*impl_, channel)) {
        voice->sound->stop();
    }
}

bool HGE::Channel_IsPlaying(HCHANNEL channel) {
    if (auto* voice = GetVoice(*impl_, channel)) {
        return voice->sound->getStatus() == sf::SoundSource::Status::Playing;
    }
    return false;
}

float HGE::Channel_GetLength(HCHANNEL channel) {
    if (auto* voice = GetVoice(*impl_, channel); voice && voice->buffer) {
        return voice->buffer->getDuration().asSeconds();
    }
    return 0.f;
}

float HGE::Channel_GetPos(HCHANNEL channel) {
    if (auto* voice = GetVoice(*impl_, channel)) {
        return voice->sound->getPlayingOffset().asSeconds();
    }
    return 0.f;
}

void HGE::Channel_SetPos(HCHANNEL channel, const float seconds) {
    if (auto* voice = GetVoice(*impl_, channel); voice && voice->buffer) {
        const auto duration = voice->buffer->getDuration().asSeconds();
        voice->sound->setPlayingOffset(sf::seconds(std::clamp(seconds, 0.f, duration)));
    }
}
