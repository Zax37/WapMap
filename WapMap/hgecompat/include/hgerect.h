/*
** Haaf's Game Engine 1.7
** Copyright (C) 2003-2007, Relish Games
** hge.relishgames.com
**
** hgeRect helper class
*/


#ifndef HGERECT_H
#define HGERECT_H


class hgeRect
{
public:
	float	x1, y1, x2, y2;

	hgeRect(float _x1, float _y1, float _x2, float _y2) {x1=_x1; y1=_y1; x2=_x2; y2=_y2; is_clean_=false; }
	hgeRect() { is_clean_=true;}

	void    Clear() { is_clean_=true;}
	bool    IsClean() const {return is_clean_;}
	void	Set(float _x1, float _y1, float _x2, float _y2) { x1=_x1; x2=_x2; y1=_y1; y2=_y2; is_clean_=false; }
	void	SetRadius(float x, float y, float r) { x1=x-r; x2=x+r; y1=y-r; y2=y+r; is_clean_=false; }
	void	Encapsulate(float x, float y);
	bool	TestPoint(float x, float y) const;
	bool	Intersect(const hgeRect *rect) const;

private:
	bool	is_clean_;
};


#endif
