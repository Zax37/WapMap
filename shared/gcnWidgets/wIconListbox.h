#ifndef WAPMAP_WICONLISTBOX_H
#define WAPMAP_WICONLISTBOX_H

#include <hgesprite.h>
#include "wListbox.h"

namespace SHR {
    class GCN_CORE_DECLSPEC IconListModel : public ListModel {
    public:
        virtual hgeSprite* getIcon(int i) = 0;
    };

    class GCN_CORE_DECLSPEC IconListBox : public ListBox {
    public:
        explicit IconListBox(IconListModel* model) : ListBox(model) {}

        unsigned int getRowHeight() const override;

        void drawRow(Graphics *graphics, int i) override;

        int getTextXOffset() const override { return 45; }
    };
}

#endif //WAPMAP_WICONLISTBOX_H
