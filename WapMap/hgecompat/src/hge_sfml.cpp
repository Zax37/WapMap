/*
** WapMap HGE compatibility layer
** HGE-derived interface portions are covered by ../LICENSE.txt.
*/
#include "hge.h"

void* HGE::Resource_Load(const char*, DWORD* size) {
    if (size) *size = 0;
    return nullptr;
}
void HGE::Resource_Free(void*) {}
