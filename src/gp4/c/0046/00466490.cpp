#include <gp4/gp4.h>
#include <gp4/d3d8.h>

#pragma pack(push, 1)
struct Unknown_466490 {
    uint8_t Unknown00[0x130];
    uint32_t Active130;
};
#pragma pack(pop)

// Releases the two COM objects whose pointer cells are passed in (only while +0x130 is set) and
// clears the cells.
// FUNCTION: GP4 0x00466490
void __fastcall FUN_00466490(Unknown_466490* self, void* /*edx*/, IUnknown_** first, IUnknown_** second) {
    if (self->Active130 != 0) {
        (*first)->Release();
        *first = nullptr;
        (*second)->Release();
        *second = nullptr;
    }
}
GP4_IMPL(0x00466490, FUN_00466490, "thiscall(ptr[0x134]:layout=vbo_00466490, ptr[4]:layout=vbcell_00466490, ptr[4]:layout=ibcell_00466490) -> void")
