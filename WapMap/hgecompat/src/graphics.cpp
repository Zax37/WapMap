#include "hge_internal.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/View.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <memory>
#include <vector>

namespace {

sf::Color ToColor(const DWORD color) {
    return {static_cast<std::uint8_t>(GETR(color)),
            static_cast<std::uint8_t>(GETG(color)),
            static_cast<std::uint8_t>(GETB(color)),
            static_cast<std::uint8_t>(GETA(color))};
}

template <typename State>
sf::RenderTarget* GetRenderTarget(State& state) {
    if (state.currentTarget) {
        return &state.currentTarget->renderTexture;
    }
    return &state.window;
}

template <typename State>
sf::Vector2u GetTargetSize(State& state) {
    if (state.currentTarget) {
        return state.currentTarget->renderTexture.getSize();
    }
    return state.window.getSize();
}

template <typename State>
void ApplyClipping(State& state) {
    auto* target = GetRenderTarget(state);
    auto view = target->getView();
    const auto size = GetTargetSize(state);
    if (state.clipWidth <= 0 || state.clipHeight <= 0 || !size.x || !size.y) {
        view.setScissor(sf::FloatRect({0.f, 0.f}, {1.f, 1.f}));
    } else {
        const auto left = std::clamp(state.clipX, 0, static_cast<int>(size.x));
        const auto top = std::clamp(state.clipY, 0, static_cast<int>(size.y));
        const auto right = std::clamp(state.clipX + state.clipWidth, left,
                                      static_cast<int>(size.x));
        const auto bottom = std::clamp(state.clipY + state.clipHeight, top,
                                       static_cast<int>(size.y));
        view.setScissor(sf::FloatRect(
            {static_cast<float>(left) / static_cast<float>(size.x),
             static_cast<float>(top) / static_cast<float>(size.y)},
            {static_cast<float>(right - left) / static_cast<float>(size.x),
             static_cast<float>(bottom - top) / static_cast<float>(size.y)}));
    }
    target->setView(view);
}

sf::BlendMode GetBlendMode(const int blend) {
    if ((blend & BLEND_COLORADD) != 0 || (blend & BLEND_ALPHABLEND) == 0) {
        return sf::BlendAdd;
    }
    return sf::BlendAlpha;
}

template <typename State>
void Flush(State& state) {
    if (!state.sceneOpen || state.batchVertices.empty()) {
        state.batchVertices.clear();
        return;
    }

    sf::RenderStates renderStates;
    renderStates.blendMode = GetBlendMode(state.batchBlend);
    renderStates.transform = state.transform;
    if (state.batchTexture && state.batchTexture->texture) {
        renderStates.texture = state.batchTexture->texture;
    }
    GetRenderTarget(state)->draw(state.batchVertices.data(), state.batchVertices.size(),
                                 state.batchPrimitive, renderStates);
    state.batchVertices.clear();
}

template <typename State>
void PrepareBatch(State& state, HTEXTURE texture, const int blend,
                  const sf::PrimitiveType primitive) {
    if (state.batchTexture != texture || state.batchBlend != blend
        || state.batchPrimitive != primitive) {
        Flush(state);
        state.batchTexture = texture;
        state.batchBlend = blend;
        state.batchPrimitive = primitive;
    }
}

bool LockPixels(TextureEntry& texture, const bool readOnly, const int left, const int top,
                const int width, const int height, DWORD** pixels) {
    if (!texture.texture || texture.locked || !pixels || (!readOnly && !texture.ownedTexture)) {
        return false;
    }

    const auto size = texture.texture->getSize();
    if (!size.x || !size.y || size.x > static_cast<unsigned>(std::numeric_limits<int>::max())
        || size.y > static_cast<unsigned>(std::numeric_limits<int>::max())) {
        return false;
    }
    const auto textureWidth = static_cast<int>(size.x);
    const auto textureHeight = static_cast<int>(size.y);
    const auto lockWidth = width == 0 ? textureWidth - left : width;
    const auto lockHeight = height == 0 ? textureHeight - top : height;
    if (left < 0 || top < 0 || lockWidth <= 0 || lockHeight <= 0
        || left > textureWidth - lockWidth || top > textureHeight - lockHeight) {
        return false;
    }

    const auto widthSize = static_cast<std::size_t>(textureWidth);
    const auto heightSize = static_cast<std::size_t>(textureHeight);
    if (widthSize > std::numeric_limits<std::size_t>::max() / heightSize) {
        return false;
    }
    const auto pixelCount = widthSize * heightSize;
    if (!texture.pixelsCached || !texture.ownedTexture) {
        if (pixelCount > std::numeric_limits<std::size_t>::max() / 4) {
            return false;
        }
        const auto image = texture.texture->copyToImage();
        const auto* rgba = image.getPixelsPtr();
        texture.lockedPixels.resize(pixelCount);
        for (std::size_t index = 0; index < pixelCount; ++index) {
            const auto offset = index * 4;
            texture.lockedPixels[index] = ARGB(rgba[offset + 3], rgba[offset],
                                               rgba[offset + 1], rgba[offset + 2]);
        }
        texture.pixelsCached = texture.ownedTexture != nullptr;
    }
    texture.lockLeft = left;
    texture.lockTop = top;
    texture.lockWidth = lockWidth;
    texture.lockHeight = lockHeight;
    texture.lockReadOnly = readOnly;
    texture.locked = true;
    *pixels = texture.lockedPixels.data()
              + static_cast<std::size_t>(top) * static_cast<std::size_t>(textureWidth)
              + static_cast<std::size_t>(left);
    return true;
}

void UnlockPixels(TextureEntry& texture) {
    if (!texture.locked) {
        return;
    }
    if (!texture.lockReadOnly && texture.texture) {
        if (texture.ownedTexture) {
            auto* ownedTexture = texture.ownedTexture.get();
            const auto textureWidth = static_cast<std::size_t>(ownedTexture->getSize().x);
            const auto updateWidth = static_cast<std::size_t>(texture.lockWidth);
            const auto updateHeight = static_cast<std::size_t>(texture.lockHeight);
            std::vector<std::uint8_t> rgba(updateWidth * updateHeight * 4);
            for (std::size_t y = 0; y < updateHeight; ++y) {
                for (std::size_t x = 0; x < updateWidth; ++x) {
                    const auto color = texture.lockedPixels[
                        (static_cast<std::size_t>(texture.lockTop) + y) * textureWidth
                        + static_cast<std::size_t>(texture.lockLeft) + x];
                    const auto offset = (y * updateWidth + x) * 4;
                    rgba[offset] = static_cast<std::uint8_t>(GETR(color));
                    rgba[offset + 1] = static_cast<std::uint8_t>(GETG(color));
                    rgba[offset + 2] = static_cast<std::uint8_t>(GETB(color));
                    rgba[offset + 3] = static_cast<std::uint8_t>(GETA(color));
                }
            }
            ownedTexture->update(rgba.data(),
                                 {static_cast<unsigned>(updateWidth), static_cast<unsigned>(updateHeight)},
                                 {static_cast<unsigned>(texture.lockLeft), static_cast<unsigned>(texture.lockTop)});
        }
    }
    if (!texture.pixelsCached) {
        texture.lockedPixels.clear();
    }
    texture.locked = false;
}

bool IsValidTextureSize(const int width, const int height) {
    return width > 0 && height > 0;
}

}

bool HGE::Gfx_BeginScene(HTARGET target) {
    auto& state = *impl_;
    if (target && (!target->renderTexture.getSize().x || !target->renderTexture.getSize().y)) {
        state.errorMessage = "Cannot begin a scene on an invalid render target.";
        return false;
    }
    if (!target && !state.window.isOpen()) {
        state.errorMessage = "Cannot begin a scene without an open window.";
        return false;
    }
    Flush(state);
    state.currentTarget = target;
    state.sceneOpen = true;
    state.batchTexture = nullptr;
    state.batchVertices.clear();
    ApplyClipping(state);
    return true;
}

void HGE::Gfx_EndScene() {
    auto& state = *impl_;
    Flush(state);
    if (state.sceneOpen && state.currentTarget) {
        state.currentTarget->renderTexture.display();
    }
    state.sceneOpen = false;
    state.currentTarget = nullptr;
    state.batchTexture = nullptr;
}

void HGE::Gfx_Clear(const DWORD color) {
    auto& state = *impl_;
    Flush(state);
    if (state.sceneOpen) {
        GetRenderTarget(state)->clear(ToColor(color));
    }
}

void HGE::Gfx_FlushBuffer() {
    Flush(*impl_);
}

void HGE::Gfx_RenderLine(const float x1, const float y1, const float x2, const float y2,
                         const DWORD color, const float) {
    auto& state = *impl_;
    if (!state.sceneOpen) {
        return;
    }
    PrepareBatch(state, nullptr, BLEND_DEFAULT, sf::PrimitiveType::Lines);
    const auto vertexColor = ToColor(color);
    state.batchVertices.emplace_back(sf::Vector2f{x1 + 0.5f, y1 + 0.5f}, vertexColor);
    state.batchVertices.emplace_back(sf::Vector2f{x2 + 0.5f, y2 + 0.5f}, vertexColor);
}

void HGE::Gfx_RenderTriple(const hgeTriple* triple) {
    auto& state = *impl_;
    if (!state.sceneOpen || !triple) {
        return;
    }
    PrepareBatch(state, triple->tex, triple->blend, sf::PrimitiveType::Triangles);
    const auto textureSize = triple->tex && triple->tex->texture
                                ? triple->tex->texture->getSize() : sf::Vector2u{};
    for (const auto& vertex : triple->v) {
        state.batchVertices.emplace_back(
            sf::Vector2f{vertex.x, vertex.y}, ToColor(vertex.col),
            sf::Vector2f{vertex.tx * static_cast<float>(textureSize.x),
                         vertex.ty * static_cast<float>(textureSize.y)});
    }
}

void HGE::Gfx_RenderQuad(const hgeQuad* quad, const bool filled) {
    auto& state = *impl_;
    if (!state.sceneOpen || !quad) {
        return;
    }
    if (!filled) {
        Gfx_RenderLine(quad->v[0].x, quad->v[0].y, quad->v[1].x, quad->v[1].y, quad->v[0].col);
        Gfx_RenderLine(quad->v[1].x, quad->v[1].y, quad->v[2].x, quad->v[2].y, quad->v[1].col);
        Gfx_RenderLine(quad->v[2].x, quad->v[2].y, quad->v[3].x, quad->v[3].y, quad->v[2].col);
        Gfx_RenderLine(quad->v[3].x, quad->v[3].y, quad->v[0].x, quad->v[0].y, quad->v[3].col);
        return;
    }

    PrepareBatch(state, quad->tex, quad->blend, sf::PrimitiveType::Triangles);
    const auto textureSize = quad->tex && quad->tex->texture
                                ? quad->tex->texture->getSize() : sf::Vector2u{};
    constexpr std::array<unsigned, 6> indices{0, 1, 2, 0, 2, 3};
    for (const auto index : indices) {
        const auto& vertex = quad->v[index];
        state.batchVertices.emplace_back(
            sf::Vector2f{vertex.x, vertex.y}, ToColor(vertex.col),
            sf::Vector2f{vertex.tx * static_cast<float>(textureSize.x),
                         vertex.ty * static_cast<float>(textureSize.y)});
    }
}

void HGE::Gfx_SetClipping(const int x, const int y, const int width, const int height) {
    auto& state = *impl_;
    Flush(state);
    state.clipX = x;
    state.clipY = y;
    state.clipWidth = width;
    state.clipHeight = height;
    if (state.sceneOpen) {
        ApplyClipping(state);
    }
}

void HGE::Gfx_GetClipping(int* x, int* y, int* width, int* height) {
    const auto& state = *impl_;
    if (x) *x = state.clipX;
    if (y) *y = state.clipY;
    if (width) *width = state.clipWidth;
    if (height) *height = state.clipHeight;
}

void HGE::Gfx_SetTransform(const float x, const float y, const float dx, const float dy,
                           const float rotation, const float hscale, const float vscale) {
    auto& state = *impl_;
    Flush(state);
    state.transform = sf::Transform::Identity;
    state.transform.translate({x + dx, y + dy});
    state.transform.rotate(sf::degrees(rotation * 180.f / static_cast<float>(M_PI)));
    state.transform.scale({hscale == 0.f ? 1.f : hscale, vscale == 0.f ? 1.f : vscale});
    state.transform.translate({-x, -y});
}

HTARGET HGE::Target_Create(const int width, const int height, const bool) {
    if (!IsValidTextureSize(width, height)) {
        impl_->errorMessage = "Render-target dimensions must be positive.";
        return nullptr;
    }
    auto target = std::make_unique<TargetEntry>();
    if (!target->renderTexture.resize(
            {static_cast<unsigned>(width), static_cast<unsigned>(height)})) {
        impl_->errorMessage = "SFML could not create the render target.";
        return nullptr;
    }
    target->texture.texture = &target->renderTexture.getTexture();
    target->surface.target = target.get();
    return target.release();
}

void HGE::Target_Free(HTARGET target) {
    if (!target) {
        return;
    }
    auto& state = *impl_;
    if (state.currentTarget == target) {
        Flush(state);
        state.sceneOpen = false;
        state.currentTarget = nullptr;
    }
    if (state.batchTexture == &target->texture) {
        Flush(state);
        state.batchTexture = nullptr;
    }
    delete target;
}

HTEXTURE HGE::Target_GetTexture(HTARGET target) {
    return target ? &target->texture : nullptr;
}

HSURFACE HGE::Target_GetSurface(HTARGET target) {
    return target ? &target->surface : nullptr;
}

DWORD* HGE::Surface_Lock(HSURFACE surface, const bool, const int left, const int top,
                         const int width, const int height) {
    if (!surface || !surface->target) {
        return nullptr;
    }
    auto& state = *impl_;
    if (state.sceneOpen && state.currentTarget == surface->target) {
        Flush(state);
    }
    const auto image = surface->target->renderTexture.getTexture().copyToImage();
    const auto size = image.getSize();
    const auto lockWidth = width == 0 ? static_cast<int>(size.x) - left : width;
    const auto lockHeight = height == 0 ? static_cast<int>(size.y) - top : height;
    if (left < 0 || top < 0 || lockWidth <= 0 || lockHeight <= 0
        || left > static_cast<int>(size.x) - lockWidth
        || top > static_cast<int>(size.y) - lockHeight) {
        return nullptr;
    }
    const auto* rgba = image.getPixelsPtr();
    const auto widthSize = static_cast<std::size_t>(size.x);
    const auto heightSize = static_cast<std::size_t>(size.y);
    if (!heightSize || widthSize > std::numeric_limits<std::size_t>::max() / heightSize) {
        return nullptr;
    }
    const auto pixelCount = widthSize * heightSize;
    if (pixelCount > std::numeric_limits<std::size_t>::max() / 4) {
        return nullptr;
    }
    surface->lockedPixels.resize(pixelCount);
    for (std::size_t index = 0; index < pixelCount; ++index) {
        const auto offset = index * 4;
        surface->lockedPixels[index] = ARGB(rgba[offset + 3], rgba[offset],
                                            rgba[offset + 1], rgba[offset + 2]);
    }
    return surface->lockedPixels.data()
           + static_cast<std::size_t>(top) * static_cast<std::size_t>(size.x)
           + static_cast<std::size_t>(left);
}

HTEXTURE HGE::Texture_Create(const int width, const int height) {
    if (!IsValidTextureSize(width, height)) {
        impl_->errorMessage = "Texture dimensions must be positive.";
        return nullptr;
    }
    auto texture = std::make_unique<TextureEntry>();
    texture->ownedTexture = std::make_unique<sf::Texture>();
    if (!texture->ownedTexture->resize(
            {static_cast<unsigned>(width), static_cast<unsigned>(height)})) {
        impl_->errorMessage = "SFML could not create the texture.";
        return nullptr;
    }
    texture->ownedTexture->setSmooth(System_GetState(HGE_TEXTUREFILTER));
    texture->texture = texture->ownedTexture.get();
    return texture.release();
}

HTEXTURE HGE::Texture_Load(const char* filename, const DWORD size, const bool mipmap) {
    if (!filename) {
        impl_->errorMessage = "Texture source cannot be null.";
        return nullptr;
    }
    auto texture = std::make_unique<TextureEntry>();
    texture->ownedTexture = std::make_unique<sf::Texture>();
    const bool loaded = size != 0
                            ? texture->ownedTexture->loadFromMemory(filename, size)
                            : texture->ownedTexture->loadFromFile(std::filesystem::path(filename));
    if (!loaded) {
        impl_->errorMessage = "SFML could not load the texture.";
        return nullptr;
    }
    texture->ownedTexture->setSmooth(System_GetState(HGE_TEXTUREFILTER));
    if (mipmap) {
        const bool generated = texture->ownedTexture->generateMipmap();
        if (!generated) {
            impl_->errorMessage = "SFML could not generate texture mipmaps.";
            return nullptr;
        }
    }
    texture->texture = texture->ownedTexture.get();
    return texture.release();
}

void HGE::Texture_Free(HTEXTURE texture) {
    if (!texture) {
        return;
    }
    auto& state = *impl_;
    if (state.batchTexture == texture) {
        Flush(state);
        state.batchTexture = nullptr;
    }
    delete texture;
}

int HGE::Texture_GetWidth(HTEXTURE texture, bool) {
    return texture && texture->texture ? static_cast<int>(texture->texture->getSize().x) : 0;
}

int HGE::Texture_GetHeight(HTEXTURE texture, bool) {
    return texture && texture->texture ? static_cast<int>(texture->texture->getSize().y) : 0;
}

DWORD* HGE::Texture_Lock(HTEXTURE texture, const bool readOnly, const int left, const int top,
                         const int width, const int height) {
    if (!texture) {
        return nullptr;
    }
    if (impl_->batchTexture == texture) {
        Flush(*impl_);
    }
    DWORD* pixels{};
    return LockPixels(*texture, readOnly, left, top, width, height, &pixels) ? pixels : nullptr;
}

void HGE::Texture_Unlock(HTEXTURE texture) {
    if (texture) {
        if (impl_->batchTexture == texture) {
            Flush(*impl_);
        }
        UnlockPixels(*texture);
    }
}
