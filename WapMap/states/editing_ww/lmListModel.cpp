#include "../editing_ww.h"
#include "../../globals.h"
#include "../../../shared/commonFunc.h"
#include "../../langID.h"
#include "../../databanks/imageSets.h"
#include "../../databanks/anims.h"
#include "../../databanks/sounds.h"
extern HGE * hge;

namespace State {
	EditingWWlModel::EditingWWlModel(EditingWW * owner, int type) {
		m_hOwn = owner;
		m_iType = type;
	}

	std::string EditingWWlModel::getElementAt(int i)
	{
		if (m_iType == LMODEL_PLANES) {
			if (m_hOwn->hParser != NULL && i >= 0 && i < m_hOwn->hParser->GetPlanesCount())
				return m_hOwn->hParser->GetPlane(i)->GetName();
			else
				return "---";
		}
		else if (m_iType == LMODEL_PLANESPROP) {
			if (m_hOwn->hParser != NULL && i >= 0 && i < m_hOwn->hParser->GetPlanesCount())
				return m_hOwn->hParser->GetPlane(i)->GetName();
			else if (i == m_hOwn->hParser->GetPlanesCount())
				return GETL2S("PlaneProperties", "NewPlane");
			else
				return "---";
		}
		else if (m_iType == LMODEL_SEARCHTERM) {
			if (i < 0 || i > 3) return "";
			char tmp[32];
			sprintf(tmp, "Term_%d", i);
			return GETL2S("ObjectSearch", tmp);
		}
		else if (m_iType == LMODEL_SCALE) {
			switch (i)
			{
			case 0: return "25%";
			case 1: return "50%";
			case 2: return "75%";
			case 3: return "100%";
			}
		}
		else if (m_iType == LMODEL_ASSETS)
			if (m_hOwn->hParser == NULL)
				return "";
			else
				if (i >= m_hOwn->SprBank->GetAssetsCount())
					return "";
				else
					return m_hOwn->SprBank->GetAssetByIterator(i)->GetID();
		else if (m_iType == LMODEL_ASSETFRAMES) {
			if (m_hOwn->hParser == NULL || m_hOwn->dbAssetsImg->getSelected() == -1) return "";
			std::string ret = m_hOwn->SprBank->GetAssetByIterator(m_hOwn->dbAssetsImg->getSelected())->GetIMGByIterator(i)->GetName();
			return ret;
		}
		else if (m_iType == LMODEL_ANIMATIONS) {
			if (m_hOwn->hParser == NULL || i < 0 || i >= m_hOwn->hAniBank->GetAssetsCount())
				return "";
			return m_hOwn->hAniBank->GetAssetByIterator(i)->GetID();
		}
		else if (m_iType == LMODEL_ANIMFRAMES) {
			if (m_hOwn->dbAssetsAni->getSelected() == -1) return "";
			ANI::Animation * ani = m_hOwn->hAniBank->GetAssetByIterator(m_hOwn->dbAssetsAni->getSelected())->GetAni();
			if (i < 0 || i >= ani->GetFramesCount()) return "";
			char tmp[64];
			if (ani->GetFrame(i)->GetKeyFrameName() != NULL)
				sprintf(tmp, "%s #%d (%s)", GETL2("WinDatabase", Lang_Db_Frame), i + 1, ani->GetFrame(i)->GetKeyFrameName());
			else
				sprintf(tmp, "%s #%d", GETL2("WinDatabase", Lang_Db_Frame), i + 1);
			return tmp;
		}
		else if (m_iType == LMODEL_IMAGESETS) {
			if (m_hOwn->hParser == NULL) return "";
			std::string ret = m_hOwn->hParser->GetImageSet(i);
			ret.append(" (");
			ret.append(m_hOwn->hParser->GetImageSetPrefix(i));
			ret.append(")");
			return ret;
		}
		else if (m_iType == LMODEL_TILETYPE) {
			if (i == 0)
				return GETL(Lang_AtribSingle);
			else
				return GETL(Lang_AtribDouble);
		}
		else if (m_iType == LMODEL_TILEATRIB) {
			switch (i)
			{
			case 0: return "Clear";
			case 1: return "Solid";
			case 2: return "Ground";
			case 3: return "Climb";
			case 4: return "Death";
			default: return "NULL";
			}
		}
		else if (m_iType == LMODEL_SOUNDS) {
			if (i == 0)
				return "-----";
			else
				if (m_hOwn->hSndBank->GetAssetByIterator(i - 1) != NULL)
					return m_hOwn->hSndBank->GetAssetByIterator(i - 1)->GetName();
				else
					return "";
		}
		else if (m_iType == LMODEL_SOUNDS_CLEAR) {
			if (m_hOwn->hParser == NULL)
				return "";
			else
				if (m_hOwn->hSndBank->GetAssetByIterator(i) != NULL)
					return m_hOwn->hSndBank->GetAssetByIterator(i)->GetName();
				else
					return "";
		}
		else if (m_iType == LMODEL_ANIMSPEED) {
			char ret[25];
			sprintf(ret, "%d%%", (i + 1) * 25);
			return ret;
		}
	}

	int EditingWWlModel::getNumberOfElements()
	{
		if (m_iType == LMODEL_PLANES) {
			if (m_hOwn->hParser != NULL)
				return m_hOwn->hParser->GetPlanesCount();
			else
				return 1;
		}
		else if (m_iType == LMODEL_SEARCHTERM)
			return 4;
		else if (m_iType == LMODEL_PLANESPROP) {
			if (m_hOwn->hParser != NULL)
				return m_hOwn->hParser->GetPlanesCount() + 1;
			else
				return 1;
		}
		else if (m_iType == LMODEL_SCALE)
			return 4;
		else if (m_iType == LMODEL_ASSETS) {
			if (m_hOwn->hParser == NULL)
				return 0;
			else
				return m_hOwn->SprBank->GetAssetsCount();
		}
		else if (m_iType == LMODEL_ASSETFRAMES) {
			if (m_hOwn->hParser == NULL || m_hOwn->dbAssetsImg->getSelected() == -1) return 0;
			int ret = m_hOwn->SprBank->GetAssetByIterator(m_hOwn->dbAssetsImg->getSelected())->GetSpritesCount();
			return ret;
		}
		else if (m_iType == LMODEL_ANIMATIONS) {
			if (m_hOwn->hParser == NULL) return 0;
			return m_hOwn->hAniBank->GetAssetsCount();
		}
		else if (m_iType == LMODEL_ANIMFRAMES) {
			if (m_hOwn->hParser == NULL || m_hOwn->dbAssetsAni->getSelected() == -1) return 0;
			return m_hOwn->hAniBank->GetAssetByIterator(m_hOwn->dbAssetsAni->getSelected())->GetAni()->GetFramesCount();
		}
		else if (m_iType == LMODEL_IMAGESETS) {
			if (m_hOwn->hParser == NULL) return 0;
			if (m_hOwn->hParser->GetImageSet(3)[0] != '\0')
				return 4;
			else if (m_hOwn->hParser->GetImageSet(2)[0] != '\0')
				return 3;
			else if (m_hOwn->hParser->GetImageSet(1)[0] != '\0')
				return 2;
			else if (m_hOwn->hParser->GetImageSet(0)[0] != '\0')
				return 1;
			else
				return 0;
		}
		else if (m_iType == LMODEL_TILETYPE)
			return 2;
		else if (m_iType == LMODEL_TILEATRIB)
			return 5;
		else if (m_iType == LMODEL_SOUNDS)
			if (m_hOwn->hParser == NULL)
				return 1;
			else
				return m_hOwn->hSndBank->GetAssetsCount() + 1;
		else if (m_iType == LMODEL_SOUNDS_CLEAR)
			if (m_hOwn->hParser == NULL)
				return 1;
			else
				return m_hOwn->hSndBank->GetAssetsCount();
		else if (m_iType == LMODEL_ANIMSPEED)
			return 8;
	}

}
