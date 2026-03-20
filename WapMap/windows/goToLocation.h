#ifndef H_WIN_GO_TO_LOCATION
#define H_WIN_GO_TO_LOCATION

#include "../cMDI.h"
#include "../../shared/gcnWidgets/wListBox.h"
#include "../../shared/gcnWidgets/wScrollArea.h"
#include "../../shared/gcnWidgets/wButton.h"
#include "../../shared/gcnWidgets/wTabbedArea.h"
#include "../../shared/gcnWidgets/wTab.h"

#include "window.h"

class LocationsList : public gcn::ListModel {
    private:
        std::vector<stLocation> *v = NULL;

        LocationsList() {};

        int addLocation(std::string& name, int x, int y);

        bool addLocationLinkedToObject(WWD::Object *object);

        void deleteLocation(int i);

        stLocation* getLocation(int i);

        int renameLocation(int index, std::string& newName);

        void sort();

        bool moveElement(int index, int move);

        bool isObjectList = 0;
    
    public:

        std::string getElementAt(int i);

        int getNumberOfElements() { return v ? v->size() : 0; };

    friend class winLocationsBrowser;
};

class winLocationsBrowser : public cWindow {
    enum Locs {
        Favourites = 0,
        Checkpoints,
        Warps,
        COUNT
    };

    private:
        SHR::ListBox *lbLocs;
        SHR::ScrollArea *saLocs;
        SHR::TabbedArea *tabarLocs;

        std::vector<stLocation> vWarps, vCheckpoints;

        LocationsList *lmLocs[Locs::COUNT];

        SHR::But *butDelete, *butEdit, *butAddFav, *butMoveDown, *butMoveUp, *butFollowWarp;

        int m_selectedTab = Locs::Favourites;

        void RefreshLists();

    public:
        winLocationsBrowser();

        void AddFavLocation(int x, int y);

        void Open() override;

        void OnDocumentChange() override;

        void Think() override;

        void action(const ActionEvent &actionEvent) override;
};

#endif // H_WIN_GO_TO_LOCATION