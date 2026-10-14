/*
** Haaf's Game Engine 1.7
** Copyright (C) 2003-2007, Relish Games
** hge.relishgames.com
**
** hgeFont helper class implementation
*/


#include "../../include/hgefont.h"
#include <cstdarg>
#include <stdio.h>

const char fnt_header_tag[] = "[HGEFONT]";
const char fnt_bitmap_tag[] = "Bitmap";
const char fnt_char_tag[] = "Char";


HGE* hgeFont::hge_ = nullptr;
char hgeFont::buffer_[1024];


hgeFont::hgeFont(const char* font, const bool mipmap) {
    DWORD size;
    char linebuf[256];
    char buf[MAX_PATH], *pbuf;
    int i, x, y, w, h, a, c;

    // Setup variables

    hge_ = hgeCreate(HGE_VERSION);

    height_ = 0.0f;
    scale_ = 1.0f;
    proportion_ = 1.0f;
    rot_ = 0.0f;
    tracking_ = 0.0f;
    spacing_ = 1.0f;
    texture_ = 0;
    delete_tex_ = 1;

    z_ = 0.5f;
    blend_ = BLEND_COLORMUL | BLEND_ALPHABLEND | BLEND_NOZWRITE;
    col_ = 0xFFFFFFFF;

    ZeroMemory( &letters_, sizeof(letters_) );
    ZeroMemory( &realw_, sizeof(realw_) );
    ZeroMemory( &pre_, sizeof(pre_) );
    ZeroMemory( &post_, sizeof(post_) );

    // Load font description

    void* data = hge_->Resource_Load(font, &size);
    if (!data) {
        return;
    }

    char* desc = new char[size + 1];
    memcpy(desc, data, size);
    desc[size] = 0;
    hge_->Resource_Free(data);

    char* pdesc = _get_line(desc, linebuf);
    if (!strncmp(linebuf, "<?xml", 5)) {
        LoadXML(desc, size, 0, 0);
        delete[] desc;
        return;
    } else if (!strncmp(linebuf, "csv", 3)) {
        LoadCSV(desc, size, 0, 0);
        delete[] desc;
        return;
    } else if (strcmp(linebuf, fnt_header_tag)) {
        hge_->System_Log("Font %s has incorrect format.", font);
        delete[] desc;
        return;
    }

    // Parse font description

    while ((pdesc = _get_line(pdesc, linebuf))) {
        if (!strncmp(linebuf, fnt_bitmap_tag, sizeof(fnt_bitmap_tag) - 1)) {
            strcpy(buf, font);
            pbuf = strrchr(buf, '\\');
            if (!pbuf) {
                pbuf = strrchr(buf, '/');
            }
            if (!pbuf) {
                pbuf = buf;
            }
            else {
                pbuf++;
            }
            if (!sscanf(linebuf, "Bitmap = %s", pbuf)) {
                continue;
            }

            texture_ = hge_->Texture_Load(buf, 0, mipmap);
            if (!texture_) {
                delete[] desc;
                return;
            }
        }

        else if (!strncmp(linebuf, fnt_char_tag, sizeof(fnt_char_tag) - 1)) {
            pbuf = strchr(linebuf, '=');
            if (!pbuf) {
                continue;
            }
            pbuf++;
            while (*pbuf == ' ') {
                pbuf++;
            }
            if (*pbuf == '\"') {
                pbuf++;
                i = static_cast<unsigned char>(*pbuf++);
                pbuf++; // skip "
            }
            else {
                i = 0;
                while ((*pbuf >= '0' && *pbuf <= '9') || (*pbuf >= 'A' && *pbuf <= 'F') || (*pbuf >=
                    'a' && *pbuf <= 'f')) {
                    char chr = *pbuf;
                    if (chr >= 'a') {
                        chr -= 'a' - ':';
                    }
                    if (chr >= 'A') {
                        chr -= 'A' - ':';
                    }
                    chr -= '0';
                    if (chr > 0xF) {
                        chr = 0xF;
                    }
                    i = (i << 4) | chr;
                    pbuf++;
                }
                if (i < 0 || i > 255) {
                    continue;
                }
            }
            sscanf(pbuf, " , %d , %d , %d , %d , %d , %d", &x, &y, &w, &h, &a, &c);

            letters_[i] = new hgeSprite(texture_,
                                        static_cast<float>(x),
                                        static_cast<float>(y),
                                        static_cast<float>(w),
                                        static_cast<float>(h));
            pre_[i] = static_cast<float>(a);
            post_[i] = static_cast<float>(c);
            realw_[i] = static_cast<float>(w);
            if (h > height_) {
                height_ = static_cast<float>(h);
            }
        }
    }

    delete[] desc;
}


hgeFont::~hgeFont() {
    for (int i = 0; i < 256; i++)
        if (letters_[i]) {
            delete letters_[i];
        }
    if (delete_tex_) {
        hge_->Texture_Free(texture_);
    }
    hge_->Release();
}

void hgeFont::Render(const float x, float y, int align, const char* string, BOOL pbDisableColors) {
    float fx = x;

    switch (align & HGETEXT_HORZMASK) {
        case HGETEXT_RIGHT:
            fx -= GetStringWidth(string, false);
            break;
        case HGETEXT_CENTER:
            fx -= int(GetStringWidth(string, false) / 2.0f);
            break;
    }

    if (align & HGETEXT_VERTMASK) {
        int lines = 1;
        std::string str(string);
        for (size_t offset = str.find("~n~"); offset != std::string::npos;
             offset = str.find("~n~", offset + 3)) {
            ++lines;
        }

        switch (align & HGETEXT_VERTMASK) {
            case HGETEXT_BOTTOM:
                y -= int(lines * GetHeight());
                break;
            case HGETEXT_MIDDLE:
                y -= int(lines * GetHeight() * 0.5f);
                break;
        }
    }

    while (*string) {
        if (string[0] == '\n'
        || (!pbDisableColors && string[1] && string[2] && string[0] == '~' && string[1] == 'n' && string[2] == '~')) {
            y += int(height_ * scale_ * spacing_);
            fx = x;
            if (align == HGETEXT_RIGHT) {
                fx -= GetStringWidth(string + 1, false);
            }
            if (align == HGETEXT_CENTER) {
                fx -= int(GetStringWidth(string + 1, false) / 2.0f);
            }
            if (string[0] != '\n') {
                string += 3;
                continue;
            }
        }
        else {
            int i = static_cast<unsigned char>(*string);

            if (string[1] && string[2] && string[0] == '~' && string[2] == '~') {
                if (!pbDisableColors) {
                    switch (string[1]) {
                        case 'a':
                            SetColor(0xFF777777);
                            string += 3;
                            continue;
                        case 'b':
                            SetColor(0xFF0000FF);
                            string += 3;
                            continue;
                        case 'g':
                            SetColor(0xFF4CD68F);
                            string += 3;
                            continue;
                        case 'l':
                            SetColor(0xFF000000);
                            string += 3;
                            continue;
                        case 'p':
                            SetColor(0xFFFF00FF);
                            string += 3;
                            continue;
                        case 'r':
                            SetColor(0xFFFF0000);
                            string += 3;
                            continue;
                        case 'w':
                            SetColor(0xFFe1e1e1);
                            string += 3;
                            continue;
                        case 'y':
                            SetColor(0xFFFFFF00);
                            string += 3;
                            continue;
                        case ' ':
                            i = ' ';
                            string += 2;
                            break;
                    }
                }
            }

            if (!letters_[i]) {
                i = '?';
            }
            if (letters_[i]) {
                fx += pre_[i] * scale_ * proportion_;
                letters_[i]->RenderEx(fx, y, rot_, scale_ * proportion_, scale_);
                fx += (realw_[i] + post_[i] + tracking_) * scale_ * proportion_;
            }
        }
        string++;
    }
}

void hgeFont::printf(const float x, const float y, const int align, 
                     const char* format, BOOL pbDisableColors, ...) {
    va_list args;
    va_start(args, pbDisableColors);
    _vsnprintf(buffer_, sizeof(buffer_) - 1, format, args);
    va_end(args);
    buffer_[sizeof(buffer_) - 1] = 0;

    Render(x, y, align, buffer_, pbDisableColors);
}

void hgeFont::printfb(const float x, const float y, const float w, const float h,
                      const int align, BOOL pbDisableColors, const char* format, ...) {
    auto lines = 0;
    va_list args;
    va_start(args, format);
    _vsnprintf(buffer_, sizeof(buffer_) - 1, format, args);
    va_end(args);
    buffer_[sizeof(buffer_) - 1] = 0;

    char* linestart = buffer_;
    char* pbuf = buffer_;
    char* prevword = nullptr;

    for (;;) {
        int i = 0;
        while (pbuf[i] && (pbuf[i] != ' ' || (pbuf[i + 1] == '~' && !pbDisableColors && i > 0 && pbuf[i - 1] == '~')) && pbuf[i] != '\n') {
            i++;
        }

        const auto chr = pbuf[i];
        pbuf[i] = 0;
        const auto ww = GetStringWidth(linestart);
        pbuf[i] = chr;

        if (ww > w) {
            if (pbuf == linestart) {
                pbuf[i] = '\n';
                linestart = &pbuf[i + 1];
            }
            else {
                *prevword = '\n';
                linestart = prevword + 1;
            }

            lines++;
        }

        if (pbuf[i] == '\n') {
            prevword = &pbuf[i];
            linestart = &pbuf[i + 1];
            pbuf = &pbuf[i + 1];
            lines++;
            continue;
        }

        if (!pbuf[i]) {
            lines++;
            break;
        }

        prevword = &pbuf[i];
        pbuf = &pbuf[i + 1];
    }

    auto tx = x;
    auto ty = y;
    const auto hh = height_ * spacing_ * scale_ * lines;

    switch (align & HGETEXT_HORZMASK) {
    case HGETEXT_LEFT:
        break;
    case HGETEXT_RIGHT:
        tx += w;
        break;
    case HGETEXT_CENTER:
        tx += int(w / 2);
        break;
    }

    switch (align & HGETEXT_VERTMASK) {
    case HGETEXT_TOP:
        break;
    case HGETEXT_BOTTOM:
        ty += h - hh;
        break;
    case HGETEXT_MIDDLE:
        ty += int((h - hh) / 2);
        break;
    }

    Render(tx, ty, align, buffer_, pbDisableColors);
}

float hgeFont::GetStringWidth(const char* string, const bool b_multiline, bool bDisableColors) const {
    float w = 0;

    while (*string) {
        float linew = 0;

        while (*string && *string != '\n') {
            int i = static_cast<unsigned char>(*string);
            if (!bDisableColors && string[1] && string[2] && string[0] == '~' && string[2] == '~') {
                if (string[1] == 'n') {
                    string += 3;
                    break;
                } else if (string[1] == ' ') {
                    i = ' ';
                    linew += realw_[i] + pre_[i] + post_[i] + tracking_;
                }

                string += 3;
                continue;
            }

            if (!letters_[i]) {
                i = '?';
            }
            if (letters_[i]) {
                linew += realw_[i] + pre_[i] + post_[i] + tracking_;
            }

            string++;
        }

        if (!b_multiline) {
            return linew * scale_ * proportion_;
        }

        if (linew > w) {
            w = linew;
        }

        while (*string == '\n' || *string == '\r') {
            string++;
        }
    }

    return w * scale_ * proportion_;
}

void hgeFont::SetColor(const DWORD col) {
    col_ = col;

    for (int i = 0; i < 256; i++)
        if (letters_[i]) {
            letters_[i]->SetColor(col);
        }
}

void hgeFont::SetZ(const float z) {
    z_ = z;

    for (auto i = 0; i < 256; i++)
        if (letters_[i]) {
            letters_[i]->SetZ(z);
        }
}

void hgeFont::SetBlendMode(const int blend) {
    blend_ = blend;

    for (auto i = 0; i < 256; i++) {
        if (letters_[i]) {
            letters_[i]->SetBlendMode(blend);
        }
    }
}

char* hgeFont::_get_line(char* file, char* line) {
    auto i = 0;

    if (!file[i]) {
        return nullptr;
    }

    while (file[i] && file[i] != '\n' && file[i] != '\r') {
        line[i] = file[i];
        i++;
    }
    line[i] = 0;

    while (file[i] && (file[i] == '\n' || file[i] == '\r')) {
        i++;
    }

    return file + i;
}

hgeFont::hgeFont(char *data, int datasize, HTEXTURE tex, bool pbDeleteTex, int iOffX, int iOffY) {
    // Setup variables

    hge_ = hgeCreate(HGE_VERSION);
    texture_ = tex;
    delete_tex_ = pbDeleteTex;
    height_ = 0.0f;
    scale_ = 1.0f;
    proportion_ = 1.0f;
    rot_ = 0.0f;
    tracking_ = 0.0f;
    spacing_ = 1.0f;
    z_ = 0.5f;
    blend_ = BLEND_COLORMUL | BLEND_ALPHABLEND | BLEND_NOZWRITE;
    col_ = 0xFFFFFFFF;

    ZeroMemory( &letters_, sizeof(letters_) );
    ZeroMemory( &pre_, sizeof(pre_) );
    ZeroMemory( &post_, sizeof(post_) );
    ZeroMemory( &realw_, sizeof(realw_) );

    char* pdesc = _get_line(data, buffer_);
    if (!strncmp(buffer_, "<?xml", 5)) {
        LoadXML(data, datasize, iOffX, iOffY);
        return;
    } else if (!strncmp(buffer_, "csv", 3)) {
        LoadCSV(data, datasize, iOffX, iOffY);
        return;
    } else if (strcmp(buffer_, fnt_header_tag) != 0) {
        hge_->System_Log("Font from memory has incorrect format.");
        return;
    }

    int i, x, y, w, h, a, c;
    // Parse font description
    while ((pdesc = _get_line(pdesc, buffer_))) {
        if (!strncmp(buffer_, fnt_char_tag, sizeof(fnt_char_tag) - 1)) {
            char* pbuf = strchr(buffer_, '=');
            if (!pbuf) {
                continue;
            }
            pbuf++;
            while (*pbuf == ' ') {
                pbuf++;
            }
            if (*pbuf == '\"') {
                pbuf++;
                i = static_cast<unsigned char>(*pbuf++);
                pbuf++; // skip "
            }
            else {
                i = 0;
                while ((*pbuf >= '0' && *pbuf <= '9') || (*pbuf >= 'A' && *pbuf <= 'F') || (*pbuf >=
                                                                                            'a' && *pbuf <= 'f')) {
                    char chr = *pbuf;
                    if (chr >= 'a') {
                        chr -= 'a' - ':';
                    }
                    if (chr >= 'A') {
                        chr -= 'A' - ':';
                    }
                    chr -= '0';
                    if (chr > 0xF) {
                        chr = 0xF;
                    }
                    i = (i << 4) | chr;
                    pbuf++;
                }
                if (i < 0 || i > 255) {
                    continue;
                }
            }
            sscanf(pbuf, " , %d , %d , %d , %d , %d , %d", &x, &y, &w, &h, &a, &c);

            letters_[i] = new hgeSprite(texture_,
                                        static_cast<float>(x),
                                        static_cast<float>(y),
                                        static_cast<float>(w),
                                        static_cast<float>(h));
            realw_[i] = static_cast<float>(w);
            pre_[i] = static_cast<float>(a);
            post_[i] = static_cast<float>(c);
            if (h > height_) {
                height_ = static_cast<float>(h);
            }
        }
    }
}

void hgeFont::LoadXML(char *data, int datasize, int iTexOffX, int iTexOffY) {
    char* pdesc = _get_line(data, buffer_);
    pdesc = _get_line(pdesc, buffer_);

    int a = 0, b = 0, c = 0, d = 0, w, h = 0;
    if (!std::strncmp(buffer_, "<[GlobalOffset", 14)) {
        sscanf(buffer_, "<[GlobalOffset %d %d %d %d]>", &a, &c, &b, &d);
        pdesc = _get_line(pdesc, buffer_);
    }
    sscanf(buffer_, "<Font size=\"%*d\" family=\"%*s height=\"%d\" %*s", &h);
    height_ = h;
    int i = 32;

    int ox, oy, r1, r2, r3, r4;
    while (pdesc = _get_line(pdesc, buffer_)) {
        if (!strncmp(buffer_, " <Char", 6)) {
            sscanf(buffer_, " <Char width=\"%d\" offset=\"%d %d\" rect=\"%d %d %d %d\" %*s", &w, &ox, &oy, &r1, &r2, &r3, &r4);
            auto letter = new hgeSprite(texture_, r1+iTexOffX, r2+iTexOffY, r3, r4);
            letter->SetHotSpot(a - ox, c - oy);
            letters_[i] = letter;
            realw_[i] = w;
            pre_[i] = 0;
            post_[i] = 0;
            i++;
            if (i == 127)
                i = 161;
        }
    }
}

void hgeFont::LoadCSV(char *data, int datasize, int iTexOffX, int iTexOffY) {
    char* pdesc = _get_line(data, buffer_);
    pdesc = _get_line(pdesc, buffer_);

    int h, o1, o2, o3, o4;
    sscanf(buffer_, "h[%d] o[%d %d %d %d]", &h, &o1, &o2, &o3, &o4);
    height_ = h;

    int d, r1, r2, r3, r4, ox, oy, w;
    while (pdesc = _get_line(pdesc, buffer_)) {
        sscanf(buffer_, "#%d r[%d %d %d %d] o[%d %d] w[%d]", &d, &r1, &r2, &r3, &r4, &ox, &oy, &w);
        auto letter = new hgeSprite(texture_, r1+iTexOffX, r2+iTexOffY, r3, r4);
        letter->SetHotSpot(o1 - ox, o2 - oy);
        letters_[d] = letter;
        realw_[d] = w;
        pre_[d] = 0;
        post_[d] = 0;
    }
}

float hgeFont::GetStringBlockWidth(int w, const char *string) const {
    strcpy(buffer_, string);
    buffer_[sizeof(buffer_) - 1] = 0;
    char* linestart = buffer_;
    char* pbuf = buffer_;
    char* prevword = nullptr;
    float maxW = 0;

    for (;;) {
        int i = 0;
        while (pbuf[i] && (pbuf[i] != ' ' || (pbuf[i + 1] == '~' && i > 0 && pbuf[i - 1] == '~')) && pbuf[i] != '\n') {
            i++;
        }

        const auto chr = pbuf[i];
        pbuf[i] = 0;
        const auto ww = GetStringWidth(linestart);
        pbuf[i] = chr;

        if (ww > w) {
            if (pbuf == linestart) {
                pbuf[i] = '\n';
                linestart = &pbuf[i + 1];

                if (ww > maxW) {
                    maxW = ww;
                }
            } else if (prevword != linestart - 1) {
                *prevword = '\n';
                linestart = prevword + 1;
                pbuf = linestart;
                continue;
            }
        } else if (ww > maxW) {
            maxW = ww;
        }

        if (pbuf[i] == '\n') {
            prevword = &pbuf[i];
            linestart = &pbuf[i + 1];
            pbuf = &pbuf[i + 1];
            continue;
        }

        if (!pbuf[i]) {
            break;
        }

        prevword = &pbuf[i];
        pbuf = &pbuf[i + 1];
    }

    return maxW;
}

float hgeFont::GetStringBlockHeight(int w, const char *str) {
    int i, j = 1;
    strcpy(buffer_, str);
    const char* v11 = buffer_;
    char* v13 = buffer_;
    char* v12 = nullptr;
    float lastWidth;
    while ( true )
    {
        while ( true )
        {
            while ( true )
            {
                for ( i = 0; ; ++i )
                {
                    if ( !v13[i] || v13[i] == ' ' || v13[i] == '\n' )
                        break;
                }
                char v4 = v13[i];
                v13[i] = 0;
                lastWidth = GetStringWidth(v11, true);
                v13[i] = v4;
                if ( lastWidth > (long double)w )
                {
                    if ( v13 == v11 )
                    {
                        v13[i] = '\n';
                        v11 = &v13[i + 1];
                    }
                    else
                    {
                        *v12 = '\n';
                        v11 = v12 + 1;
                    }
                    ++j;
                }
                if ( v13[i] != '\n' )
                    break;
                v12 = &v13[i];
                v11 = &v13[i + 1];
                v13 += i + 1;
                ++j;
            }
            if ( v13[i] != '~' || v13[i + 1] != 'n' || v13[i + 2] != '~' )
                break;
            v12 = &v13[i];
            v11 = &v13[i + 1];
            v13 += i + 1;
            ++j;
        }
        if ( !v13[i] )
            break;
        v12 = &v13[i];
        v13 += i + 1;
    }
    return height_ * spacing_ * scale_ * (float)j;
}
