#ifndef H_C_RENDERABLEITEM
#define H_C_RENDERABLEITEM

namespace WWD {
    class Plane;

    class Object;
};

class cRenderableItem {
public:
    enum Type {
        Unknown = 0,
        PlaneWrapper,
        ObjectWrapper
    };

    cRenderableItem() = default;

    virtual ~cRenderableItem() = default;

    virtual int GetZ() { return 0; };

    virtual int GetPriority() { return 0; };

    virtual int Render() { return 0; };

    virtual int GetUniqueOrderID() { return 0; };

    virtual cRenderableItem::Type GetType() { return Unknown; };
};

bool RenderableItemSortZ(cRenderableItem *a, cRenderableItem *b);

class cPlaneRenderWrapper : public cRenderableItem {
private:
    int iIndex;
    WWD::Plane *hPlane;
public:
    cPlaneRenderWrapper(WWD::Plane *h, int iIndex);

    ~cPlaneRenderWrapper();

    virtual int GetZ();

    virtual int GetPriority();

    virtual int Render();

    virtual int GetUniqueOrderID();

    virtual cRenderableItem::Type GetType() { return PlaneWrapper; };
};

class cObjectRenderWrapper : public cRenderableItem {
private:
    WWD::Object *hObject;
public:
    cObjectRenderWrapper(WWD::Object *h);

    ~cObjectRenderWrapper();

    virtual int GetZ();

    virtual int GetPriority();

    virtual int Render();

    virtual int GetUniqueOrderID();

    virtual cRenderableItem::Type GetType() { return ObjectWrapper; };
};

#endif // H_C_RENDERABLEITEM
