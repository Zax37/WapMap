#ifndef H_RETURNCODES
#define H_RETURNCODES

#include <cstdint>

enum class ReturnCodeType : unsigned char {
    Unset = 0,
    LoadMap,
    DialogState,
    MapShot,
    ObjectProp,
    ObjPropSelectedValues
};

struct ReturnCode {
    ReturnCodeType type = ReturnCodeType::Unset;
    std::intptr_t value = 0;

    operator int() const { return static_cast<int>(value); }
};

#endif
