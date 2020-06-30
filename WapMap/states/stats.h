#ifndef H_C_STATS
#define H_C_STATS

#include "../../shared/cStateMgr.h"
#include "../../shared/cWWD.h"
#include "../../shared/gcnWidgets/wWin.h"
#include "../../shared/gcnWidgets/wLabel.h"
#include "../../shared/gcnWidgets/wTabbedArea.h"
#include "../../shared/cPieChart.h"

namespace State {
    class MapStats;

    class cMapStatsAL : public gcn::ActionListener {
    private:
        MapStats *m_hOwn;
    public:
        void action(const gcn::ActionEvent &actionEvent);

        cMapStatsAL(MapStats *owner) { m_hOwn = owner; };
    };

    struct PlaneStat {
        SHR::Container *conPl;
        SHR::cPieChart *tileTypes;
    };

    class MapStats : public SHR::cState {
    public:
        MapStats(WWD::Parser *hp);

        virtual bool Opaque() { return 0; };

        virtual void Init();

        virtual void Destroy();

        virtual bool Think();

        virtual bool Render();
        //virtual void GainFocus(ReturnCode code);

        WWD::Parser *hMap;
        gcn::Gui *gui;
        SHR::Container *conMain;
        SHR::Win *winStat;
        SHR::TabbedArea *tabbedArea;
        SHR::Container *conGen, *conObjects;

        SHR::Lab *labTreasures, *labMapName, *labMapAuthor, *labMapDate, *labMapObjectCount, *labMapPerfectScore, *labMapLayersCount;

        PlaneStat *statsPl;
        int iObjCount, iTreasureCount, iPerfectNeed;

        SHR::cPieChart *pieTreasures;

        bool bKill;
        cMapStatsAL *hAL;
        //friend class LoadMapActionListener;
    };

};


#endif
