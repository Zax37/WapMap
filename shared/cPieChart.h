#ifndef H_C_PIECHART
#define H_C_PIECHART

#include "hge.h"
#include "hgeSprite.h"
#include <vector>

namespace SHR
{
	struct cPieChartEl {
		char * szDesc;
		int iPart;
		DWORD iColor;
		float fPercentage;
	};

	class cPieChart {
	private:
		bool bDarkShadow;
		DWORD dwBg;
		int iDepthHeight;
		hgeSprite * hSpr;
		HTEXTURE hTex;
		std::vector<cPieChartEl> vElements;
		void Cleanup();
	public:
		cPieChart(int depth = 16, bool darksh = 1);
		~cPieChart();
		void Render(int iW, int iH = -1);
		hgeSprite * GetSprite() { return hSpr; };
		void AddPart(const char * szDesc, int iPart, DWORD iColor = 0);
		void SetBgCol(DWORD n) { dwBg = n; };
		int GetPartsCount() { return vElements.size(); };
		cPieChartEl GetPart(int i) { return vElements[i]; };
	};
}

#endif
