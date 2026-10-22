#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00459750 {
    uint32_t Block00[8];
    uint32_t Block20[8];
};
#pragma pack(pop)

// Clears the two 8-dword blocks at +0 and +0x20 and returns this.
// FUNCTION: GP4 0x00459750
Unknown_00459750* __fastcall FUN_00459750(Unknown_00459750* self, void* /*edx*/) {
    for (int i = 0; i < 8; i++) {
        self->Block00[i] = 0;
    }
    for (int i = 0; i < 8; i++) {
        self->Block20[i] = 0;
    }
    return self;
}
GP4_IMPL(0x00459750, FUN_00459750, "thiscall(ptr[0x40]:bytes) -> u32")
