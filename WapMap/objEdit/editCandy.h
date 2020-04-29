#ifndef H_C_OE_CANDY
#define H_C_OE_CANDY

#include "../states/objedit.h"
#include "../../shared/gcnWidgets/wListbox.h"
#include "../../shared/gcnWidgets/wTab.h"
#include "../../shared/gcnWidgets/wTextDropDown.h"

class cBankImageSet;
class cSprBankAsset;
class cSprBankAssetIMG;

namespace ObjEdit {

	class WindowMouseListener;

    class cEditObjCandy : public cObjEdit, gcn::ListModel, gcn::MouseListener {
    private:
		SHR::CBox *highDetail, *animated;
		SHR::Lab *labZPos;
        SHR::RadBut *rbType[3];
        SHR::TextField *zCoord;
		SHR::TextDropDown *animation;

		SHR::TabbedArea *tabs;
		SHR::Container *imgsCon;
		SHR::ScrollArea *saImgPick;
		cSprBankAsset *asImageSetPick = NULL, *asImageSetHover = NULL;
		cSprBankAssetIMG *asFramePick = NULL, *asFrameHover = NULL;

		std::vector<cSprBankAsset*> standardImgs, customImgs, otherImgs;
		enum TAB { TAB_STANDARD = 0, TAB_CUSTOM, TAB_OTHER };

		static std::vector<std::string> AnimationsList;

		std::function<void()> updateDimensions;

		void UpdateLogic();
		std::vector<cSprBankAsset*>& CurrentlyDisplayedImgSet();
		int CountActualImagesToDisplay(std::vector<cSprBankAsset*>& assets);
    public:
        cEditObjCandy(WWD::Object *obj, State::EditingWW *st);

        ~cEditObjCandy();

		//inherited

        virtual void Action(const gcn::ActionEvent &actionEvent);

        //virtual void Think(bool bMouseConsumed) override;
		virtual void Draw() override;

		virtual void mouseClicked(MouseEvent& mouseEvent);

		std::string getElementAt(int i) { return AnimationsList[i]; }

		int getNumberOfElements() { return AnimationsList.size(); }

		friend class cEditCandyVP;
		friend class cObjPropVP;
		friend class TabChangeListener;
		friend class WindowMouseListener;
    };
}

#endif
