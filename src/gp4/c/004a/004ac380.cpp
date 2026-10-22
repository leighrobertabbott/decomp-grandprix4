#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004ac380 {
    uint8_t Unknown00[0x18];
    int32_t Field18;
};
#pragma pack(pop)

// Clears the dword at this+0x18 and returns this (eax = ecx).
// FUNCTION: GP4 0x004ac380
Unknown_004ac380* __fastcall FUN_004ac380(Unknown_004ac380* self, void* /*edx*/) {
    self->Field18 = 0;
    return self;
}
GP4_IMPL(0x004ac380, FUN_004ac380, "thiscall(ptr[0x20]:bytes) -> u32")
