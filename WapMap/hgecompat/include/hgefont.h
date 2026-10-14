/*
** Haaf's Game Engine 1.7
** Copyright (C) 2003-2007, Relish Games
** hge.relishgames.com
**
** hgeFont helper class header
*/


#ifndef HGEFONT_H
#define HGEFONT_H


#include "hge.h"
#include "hgesprite.h"


#define HGETEXT_LEFT		0
#define HGETEXT_RIGHT		1
#define HGETEXT_CENTER		2
#define HGETEXT_HORZMASK	0x03

#define HGETEXT_TOP			0
#define HGETEXT_BOTTOM		4
#define HGETEXT_MIDDLE		8
#define HGETEXT_VERTMASK	0x0C

/*
** HGE Font class
*/
class hgeFont
{
public:
	hgeFont(const char *filename, bool bMipmap=false);
	hgeFont(char *data, int datasize, HTEXTURE tex, bool pbDeleteTex, int iOffX, int iOffY);
	~hgeFont();

	void		Render(float x, float y, int align, const char *string, BOOL pbDisableColors = 0);
	void		printf(float x, float y, int align, const char *format, BOOL pbDisableColors = 0, ...);
	void		printfb(float x, float y, float w, float h, int align, BOOL pbDisableColors, const char *format, ...);

	void		SetColor(DWORD col);
	void		SetZ(float z);
	void		SetBlendMode(int blend);
	void		SetScale(float scale) { scale_=scale;}
	void		SetProportion(float prop) { proportion_=prop; }
	void		SetRotation(float rot) { rot_=rot;}
	void		SetTracking(float tracking) { tracking_=tracking;}
	void		SetSpacing(float spacing) { spacing_=spacing;}

	DWORD		GetColor() const {return col_;}
	float		GetZ() const {return z_;}
	int			GetBlendMode() const {return blend_;}
	float		GetScale() const {return scale_;}
	float		GetProportion() const { return proportion_; }
	float		GetRotation() const {return rot_;}
	float		GetTracking() const {return tracking_;}
	float		GetSpacing() const {return spacing_;}

	hgeSprite*	GetSprite(char chr) const { return letters_[(unsigned char)chr]; }
	float		GetPreWidth(char chr) const { return pre_[(unsigned char)chr]; }
	float		GetPostWidth(char chr) const { return post_[(unsigned char)chr]; }
	float		GetHeight() const { return height_; }
	float		GetStringWidth(const char *string, bool bMultiline=true, bool bDisableColors=false) const;
	float		GetStringBlockWidth(int w, const char *string) const;
    float       GetStringBlockHeight(int w, const char * str);

private:
	hgeFont(const hgeFont &fnt);
	hgeFont&	operator= (const hgeFont &fnt);

	char*		_get_line(char *file, char *line);

	static HGE	*hge_;

	static char	buffer_[1024];

	HTEXTURE	texture_;
	hgeSprite*	letters_[256];
	float       realw_[256];
	float		pre_[256];
	float		post_[256];
	float		height_;
	float		scale_;
	float		proportion_;
	float		rot_;
	float		tracking_;
	float		spacing_;

	DWORD		col_;
	float		z_;
	int			blend_;
	bool        delete_tex_;

	void LoadXML(char * data, int datasize, int iTexOffX = 0, int iTexOffY = 0);
	void LoadCSV(char * data, int datasize, int iTexOffX = 0, int iTexOffY = 0);
};


#endif
