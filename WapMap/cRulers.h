#ifndef H_C_RULERS
#define H_C_RULERS

class cRulers {
private:
    bool bVisible;
public:
    cRulers();

    ~cRulers();

    void SetVisible(bool b);

    bool IsVisible() { return bVisible; };

    void Think();

    void Render();
};

#endif
