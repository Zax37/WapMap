#ifndef H_RETURNCODES
#define H_RETURNCODES

enum class ReturnCodeType : unsigned char {
    Unset = 0,
    LoadMap,
    DialogState,
    MapShot,
    ObjectProp,
    ObjPropSelectedValues
};

struct ReturnCode_base {
    ReturnCodeType type;
    int value;

    operator int() { return value; }
};

template<class Data>
struct ReturnCode : ReturnCode_base {
    Data data;
    operator const Data&() { return data; }
};

union PossibleReturnData {
    std::string text;

    PossibleReturnData() {}

    ~PossibleReturnData() {}
};

template<>
struct ReturnCode<PossibleReturnData> : ReturnCode_base {
    PossibleReturnData data;

    template <class T>
    ReturnCode<PossibleReturnData>& operator=(ReturnCode<T> other) {
        memcpy(this, &other, sizeof(other));
        return *this;
    }
};

template<>
struct ReturnCode<void> : ReturnCode_base {
    ReturnCode() {
        type = ReturnCodeType::Unset;
        value = 0;
    }

    ReturnCode(ReturnCodeType type, int value) {
        this->type = type;
        this->value = value;
    }

    ReturnCode(const ReturnCode<PossibleReturnData>& other) {
        memcpy(this, &other, sizeof(ReturnCode_base));
    }
};

typedef ReturnCode<std::string> TextReturnCode;
typedef ReturnCode<PossibleReturnData> BiggestReturnCode;

#endif
