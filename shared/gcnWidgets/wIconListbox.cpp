#include "wIconListbox.h"

unsigned int SHR::IconListBox::getRowHeight() const {
    return ListBox::getRowHeight() + 32;
}

void SHR::IconListBox::drawRow(Graphics *graphics, int i) {
    ListBox::drawRow(graphics, i);

    int dx, dy;
    getAbsolutePosition(dx, dy);

    unsigned rowHeight = getRowHeight();
    ((IconListModel*)mListModel)->getIcon(i)->Render(dx + 4, dy + i * rowHeight + 8);
}
