#include "objQuadTree.h"

#include "globals.h"
#include "cObjectUserData.h"
#include "databanks/imageSets.h"

cObjectQuadTree::cObjectQuadTree(WWD::Plane * owner, cBankImageSet * bank)
{
	m_hBank = bank;
	iTypicalCellW = owner->GetPlaneWidthPx();
	iTypicalCellH = owner->GetPlaneHeightPx();
	while (!(iTypicalCellW < QUAD_CELL_DIM || iTypicalCellH < QUAD_CELL_DIM)) {
		iTypicalCellW /= 2;
		iTypicalCellH /= 2;
	}

	m_hMainParent = this;
	Init(owner, 0, 0, owner->GetPlaneWidthPx(), owner->GetPlaneHeightPx());
	m_hMainParent = NULL;
	Fill(owner);
}

cObjectQuadTree::cObjectQuadTree(WWD::Plane * owner, int x, int y, int w, int h, cObjectQuadTree * parent)
{
	m_hMainParent = parent;
	Init(owner, x, y, w, h);
}

cObjectQuadTree * cObjectQuadTree::GetCellByCoords(int x, int y)
{
	if (m_bIsContainer)
		return this;

	if (x < m_iX ||
		x > m_iX + m_iCellW * 2 ||
		y < m_iY ||
		y > m_iY + m_iCellH * 2) return NULL;
	int lx = x - m_iX, ly = y - m_iY;
	if (lx < m_iCellW && ly < m_iCellH)
		return m_hCells[0]->GetCellByCoords(x, y);
	else if (lx > m_iCellW && ly < m_iCellH)
		return m_hCells[1]->GetCellByCoords(x, y);
	else if (lx > m_iCellW && ly > m_iCellH)
		return m_hCells[2]->GetCellByCoords(x, y);
	else if (lx < m_iCellW && ly > m_iCellH)
		return m_hCells[3]->GetCellByCoords(x, y);
	return NULL;
}

void cObjectQuadTree::AddObject(WWD::Object * obj)
{
	if (!m_bIsContainer) return;
	m_vObjects.push_back(obj);
	GetUserDataFromObj(obj)->ReferenceCell(this);
}

void cObjectQuadTree::DeleteObject(WWD::Object * obj)
{
	if (!m_bIsContainer) return;
	for (int i = 0; i < m_vObjects.size(); i++)
		if (m_vObjects[i] == obj) {
			m_vObjects.erase(m_vObjects.begin() + i);
			i--;
		}
}

void cObjectQuadTree::UpdateObject(WWD::Object * obj)
{
	if (m_hMainParent != NULL) { m_hMainParent->UpdateObject(obj); return; }

	//float x[4], y[4];
	//GetObjectVertices(&x[0], &y[0]);
	cObjectQuadTree * cells[4];
	GetObjectCells(obj, cells);
	cObjUserData * ud = GetUserDataFromObj(obj);
	ud->ClearCellReferences();
	for (int i = 0; i < 4; i++)
		if (cells[i] != NULL)
			cells[i]->AddObject(obj);
}

void cObjectQuadTree::GetObjectCells(WWD::Object * obj, cObjectQuadTree * cells[4])
{
	//float posx = obj->GetParam(WWD::Param_LocationX), posy = obj->GetParam(WWD::Param_LocationY);
	float posx = GetUserDataFromObj(obj)->GetX(), posy = GetUserDataFromObj(obj)->GetY();

	cSprBankAsset * asset = GetBank()->GetAssetByID(obj->GetImageSet());
	int iw, ih;
	float ihx, ihy;

	if (asset == NULL || asset->GetSpritesCount() == 0) {
		iw = GV->sprSmiley->GetWidth();
		ih = GV->sprSmiley->GetHeight();
		GV->sprSmiley->GetHotSpot(&ihx, &ihy);
	}
	else {
		int sprcount = asset->GetSpritesCount();
		int maxw = 0, maxh = 0;
		for (int z = 0; z < sprcount; z++) {
			if (asset->GetIMGByIterator(z)->GetSprite()->GetWidth() > maxw) {
				maxw = asset->GetIMGByIterator(z)->GetSprite()->GetWidth();
				float tmp;
				asset->GetIMGByIterator(z)->GetSprite()->GetHotSpot(&ihx, &tmp);
			}
			if (asset->GetIMGByIterator(z)->GetSprite()->GetHeight() > maxh) {
				maxh = asset->GetIMGByIterator(z)->GetSprite()->GetHeight();
				float tmp;
				asset->GetIMGByIterator(z)->GetSprite()->GetHotSpot(&tmp, &ihy);
			}
		}
		iw = maxw;
		ih = maxh;
	}

	//hgeSprite * spr = bank->GetObjectSprite(obj);
	float x[4], y[4];

	float hsx, hsy;
	hsx = ihx;
	hsy = ihy;
	//spr->GetHotSpot(&hsx, &hsy);
	float sprw = iw / 2, sprh = ih / 2;
	hsx -= sprw;
	hsy -= sprh;
	x[0] = posx - sprw - hsx;
	y[0] = posy - sprw - hsx;
	x[1] = posx + sprw - hsx;
	y[1] = posy - sprw - hsx;
	x[2] = posx + sprw - hsy;
	y[2] = posy + sprw - hsy;
	x[3] = posx - sprw - hsx;
	y[3] = posy + sprw - hsx;

	//cObjectQuadTree * cells[4];
	for (int z = 0; z < 4; z++) {
		cells[z] = GetCellByCoords(x[z], y[z]);
	}

	for (int z = 0; z < 4; z++)
		for (int y = 0; y < 4; y++)
			if (cells[z] == cells[y] && z != y)
				cells[z] = NULL;
}

void cObjectQuadTree::Fill(WWD::Plane * owner)
{
	for (int i = 0; i < owner->GetObjectsCount(); i++) {
		WWD::Object * obj = owner->GetObjectByIterator(i);
		//float x[4], y[4];
		//GetObjectVertices(obj, &x[0], &y[0]);

		cObjectQuadTree * cells[4];
		GetObjectCells(obj, cells);
		/*for(int z=0;z<4;z++){
		 cells[z] = GetCellByCoords(x[z], y[z]);
		}

		for(int z=0;z<4;z++)
		 for(int y=0;y<4;y++)
		  if( cells[z] == cells[y] && z != y )
		   cells[z] = NULL;*/

		for (int z = 0; z < 4; z++)
			if (cells[z] != NULL)
				cells[z]->AddObject(obj);
		//cells[z]->m_vObjects.push_back(obj);

	  /*if( (x1 > m_iX && x1 < m_iX+m_iCellW && y1 > m_iY && y1 < m_iY+m_iCellH) ||
		  (x2 > m_iX && x2 < m_iX+m_iCellW && y1 > m_iY && y1 < m_iY+m_iCellH) ||
		  (x2 > m_iX && x2 < m_iX+m_iCellW && y2 > m_iY && y2 < m_iY+m_iCellH) ||
		  (x1 > m_iX && x1 < m_iX+m_iCellW && y2 > m_iY && y2 < m_iY+m_iCellH) ){
	   m_vObjects.push_back(obj);*/
	}
}

void cObjectQuadTree::Init(WWD::Plane * owner, int x, int y, int w, int h)
{
	m_iX = x;
	m_iY = y;
	if (w < QUAD_CELL_DIM || h < QUAD_CELL_DIM) {
		m_bIsContainer = 1;
		m_iCellW = w;
		m_iCellH = h;
		for (int i = 0; i < 4; i++)
			m_hCells[i] = 0;
	}
	else {
		m_bIsContainer = 0;
		m_iCellW = w / 2;
		m_iCellH = h / 2;
		m_hCells[0] = new cObjectQuadTree(owner, x, y, m_iCellW, m_iCellH, m_hMainParent);
		m_hCells[1] = new cObjectQuadTree(owner, x + m_iCellW, y, m_iCellW, m_iCellH, m_hMainParent);
		m_hCells[2] = new cObjectQuadTree(owner, x + m_iCellW, y + m_iCellH, m_iCellW, m_iCellH, m_hMainParent);
		m_hCells[3] = new cObjectQuadTree(owner, x, y + m_iCellH, m_iCellW, m_iCellH, m_hMainParent);
	}
}

cObjectQuadTree::~cObjectQuadTree()
{
	if (!m_bIsContainer) {
		for (int i = 0; i < 4; i++)
			delete m_hCells[i];
	}
}

WWD::Object * cObjectQuadTree::GetObjectByWorldPosition(int x, int y)
{
	if (m_bIsContainer) {
		if (x < m_iX ||
			x > m_iX + m_iCellW ||
			y < m_iY ||
			y > m_iY + m_iCellH) return NULL;

		for (int i = 0; i < m_vObjects.size(); i++) {
			WWD::Rect box = GetBank()->GetObjectRenderRect(m_vObjects[i]);
			if (x > box.x1 &&
				x < box.x1 + box.x2 &&
				y > box.y1 &&
				y < box.y1 + box.y2)
				return m_vObjects[i];
		}
		return NULL;
	}
	else {
		if (x < m_iX ||
			x > m_iX + m_iCellW * 2 ||
			y < m_iY ||
			y > m_iY + m_iCellH * 2) return NULL;
		int lx = x - m_iX, ly = y - m_iY;
		if (lx < m_iCellW && ly < m_iCellH)
			return m_hCells[0]->GetObjectByWorldPosition(x, y);
		else if (lx > m_iCellW && ly < m_iCellH)
			return m_hCells[1]->GetObjectByWorldPosition(x, y);
		else if (lx > m_iCellW && ly > m_iCellH)
			return m_hCells[2]->GetObjectByWorldPosition(x, y);
		else if (lx < m_iCellW && ly > m_iCellH)
			return m_hCells[3]->GetObjectByWorldPosition(x, y);
	}
}

bool cObjectQuadTree::RectsCollideOrOverlap(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
	bool collide = 0, overlap = 0;
	if (x1 > x2 && x1    < x2 + w2 && y1    > y2 && y1    < y2 + h2 ||
		x1 + w1 > x2 && x1 + w1 < x2 + w2 && y1    > y2 && y1    < y2 + h2 ||
		x1 + w1 > x2 && x1 + w1 < x2 + w2 && y1 + h1 > y2 && y1 + h1 < y2 + h2 ||
		x1    > x2 && x1    < x2 + w2 && y1 + h1 > y2 && y1 + h1 < y2 + h2)
		collide = 1;

	int minx, miny, maxx, maxy;
	if (x1 < x2) {
		minx = x1;
		maxx = x2 + w2;
	}
	else {
		minx = x2;
		maxx = x1 + w1;
	}
	if (y1 < y2) {
		miny = y1;
		maxy = y2 + h2;
	}
	else {
		miny = y2;
		maxy = y1 + h1;
	}

	if (!((maxx - minx) > (w1 + w2) || (maxy - miny) > (h1 + h2)))
		overlap = 1;

	if (overlap || collide) return 1;
	else return 0;
}

std::vector<WWD::Object*> cObjectQuadTree::GetObjectsByArea(int x, int y, int w, int h)
{
	if (m_bIsContainer) {
		/*if( x < m_iX ||
			x > m_iX+m_iCellW ||
			y < m_iY ||
			y > m_iY+m_iCellH ) return NULL;*/
		std::vector<WWD::Object*> ret;

		for (int i = 0; i < m_vObjects.size(); i++) {
			WWD::Object * obj = m_vObjects[i];

			WWD::Rect box = GetBank()->GetObjectRenderRect(obj);

			if (RectsCollideOrOverlap(x, y, w, h, box.x1, box.y1, box.x2, box.y2))
				ret.push_back(obj);
		}
		return ret;
	}
	else {
		std::vector<WWD::Object*> ret[3], mainret;
		if (RectsCollideOrOverlap(x, y, w, h, m_iX, m_iY, m_iCellW, m_iCellH))
			mainret = m_hCells[0]->GetObjectsByArea(x, y, w, h);
		if (RectsCollideOrOverlap(x, y, w, h, m_iX + m_iCellW, m_iY, m_iCellW, m_iCellH))
			ret[0] = m_hCells[1]->GetObjectsByArea(x, y, w, h);
		if (RectsCollideOrOverlap(x, y, w, h, m_iX + m_iCellW, m_iY + m_iCellH, m_iCellW, m_iCellH))
			ret[1] = m_hCells[2]->GetObjectsByArea(x, y, w, h);
		if (RectsCollideOrOverlap(x, y, w, h, m_iX, m_iY + m_iCellH, m_iCellW, m_iCellH))
			ret[2] = m_hCells[3]->GetObjectsByArea(x, y, w, h);
		for (int z = 0; z < 3; z++) {
			for (int i = 0; i < ret[z].size(); i++) {
				mainret.push_back(ret[z][i]);
			}
		}
		for (int a = 0; a < mainret.size(); a++) {
			for (int b = 0; b < mainret.size(); b++) {
				if (mainret[a] == mainret[b] && a != b) {
					mainret.erase(mainret.begin() + b);
					if (a >= b) a--;
					b--;
				}
			}
		}
		return mainret;
	}
}

int cObjectQuadTree::GetContainerCellWidth()
{
	if (m_hMainParent == NULL)
		return iTypicalCellW;
	return m_hMainParent->GetContainerCellWidth();
}

int cObjectQuadTree::GetContainerCellHeight()
{
	if (m_hMainParent == NULL)
		return iTypicalCellH;
	return m_hMainParent->GetContainerCellHeight();
}
