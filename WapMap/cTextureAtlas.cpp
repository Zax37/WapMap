#include "cTextureAtlas.h"
#include <algorithm>

extern HGE *hge;

bool cTextureAtlaser_SortY(hgeSprite *a, hgeSprite *b) {
    return (a->GetHeight() < b->GetHeight());
}

cTextureAtlas::cTextureAtlas() {
    dimX = 2048;
    dimY = 2048;
    carX = carY = 0;
    rowH = 0;
    hTex = hge->Texture_Create(2048, 2048);
}

cTextureAtlas::~cTextureAtlas() {
    hge->Texture_Free(hTex);
}

bool cTextureAtlas::AddSprite(hgeSprite *spr) {
    if (carX + spr->GetWidth() + 2 > dimX) {
        carY += rowH + 2;
        rowH = 0;
        carX = 0;
    }
    if (carY + spr->GetHeight() + 2 > dimY)
        return false;
    if (spr->GetHeight() + 2 > rowH) rowH = spr->GetHeight() + 2;
    spr->SetTexture(hTex);
    spr->SetTextureRect(carX + 1, carY + 1, spr->GetWidth(), spr->GetHeight());
    carX += spr->GetWidth() + 2;
    vSprites.push_back(spr);
    return true;
}

bool cTextureAtlas::DeleteSprite(hgeSprite *spr) {
    for (size_t i = 0; i < vSprites.size(); i++)
        if (vSprites[i] == spr) {
            vSprites.erase(vSprites.begin() + i);
            return true;
        }
    return false;
}


cTextureAtlaser::cTextureAtlaser() {
    vAtlases.push_back(new cTextureAtlas());
}

cTextureAtlaser::~cTextureAtlaser() {
    while (!vAtlases.empty()) {
        delete vAtlases[0];
        vAtlases.erase(vAtlases.begin());
    }
}

void cTextureAtlaser::AddSprite(hgeSprite *spr) {
    vPendingSprites.push_back(spr);
}

void cTextureAtlaser::DeleteSprite(hgeSprite *spr) {
    for (size_t i = 0; i < vAtlases.size(); i++)
        if (vAtlases[i]->DeleteSprite(spr)) {
            if (vAtlases[i]->GetSpritesCount() == 0)
                vAtlases.erase(vAtlases.begin() + i);
            return;
        }
}

void cTextureAtlaser::Pack() {
    sort(vPendingSprites.begin(), vPendingSprites.end(), cTextureAtlaser_SortY);
    while (!vPendingSprites.empty()) {
        if (vAtlases.back()->AddSprite(vPendingSprites.front()))
            vPendingSprites.erase(vPendingSprites.begin());
        else
            vAtlases.push_back(new cTextureAtlas());
    }
}
