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
    if (carX + spr->GetWidth() > dimX) {
        carY += rowH;
        rowH = 0;
        carX = 0;
    }
    if (carY + spr->GetHeight() > dimY)
        return 0;
    if (spr->GetHeight() > rowH) rowH = spr->GetHeight();
    spr->SetTexture(hTex);
    spr->SetTextureRect(carX, carY, spr->GetWidth(), spr->GetHeight());
    carX += spr->GetWidth();
    vSprites.push_back(spr);
    return 1;
}

bool cTextureAtlas::DeleteSprite(hgeSprite *spr) {
    for (size_t i = 0; i < vSprites.size(); i++)
        if (vSprites[i] == spr) {
            vSprites.erase(vSprites.begin() + i);
            return 1;
        }
    return 0;
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
