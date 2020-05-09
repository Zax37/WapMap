#ifndef H_C_INVENTORYCONTROLLER
#define H_C_INVENTORYCONTROLLER

#include <string>
#include <utility>

typedef std::pair<std::string, int> cInventoryItem;

#define GetInventoryItemImageSet(x) (x.first.c_str())
#define GetInventoryItemID(x)   (x.second)

#define InventoryItemsCount 58

class cInventoryController {
private:
    cInventoryItem hElements[InventoryItemsCount];
    cInventoryItem hClipboard;
    int iDragOffX, iDragOffY;
    float fAnimTimer;
    int iAnimFrame;
public:
    cInventoryController();

    ~cInventoryController();

    cInventoryItem GetItemByID(int id);

    cInventoryItem GetItemByIt(int i);

    void SetItemInClipboard(cInventoryItem it, int offx, int offy);

    cInventoryItem GetItemInClipboard();

    void SetClipboardEmpty();

    bool IsClipboardEmpty();

    int GetAnimFrame() { return iAnimFrame; };

    void MapSwitch();

    void DrawDraggedObject();
};

#endif
