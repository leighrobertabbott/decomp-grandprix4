#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005a9cf0 {
    uint8_t Unknown00[0x71];
    uint8_t Active71;
    uint16_t Next72;
};
#pragma pack(pop)

// Hands out the next slot index (0..0x20) through `out` and advances the counter at +0x72.
// Returns 0 on success, 0x1f8 when the object is not active (+0x71 == 0) and 0x1f9 when all 0x21
// indices have been given out. `out` is set to 0xffff first.
// FUNCTION: GP4 0x005a9cf0
int __fastcall FUN_005a9cf0(Unknown_005a9cf0* self, void* /*edx*/, uint16_t* out) {
    *out = 0xffff;
    if (self->Active71 == 0) {
        return 0x1f8;
    }
    if (self->Next72 > 0x20) {
        return 0x1f9;
    }
    *out = self->Next72;
    self->Next72 = (uint16_t)(self->Next72 + 1);
    return 0;
}
GP4_IMPL(0x005a9cf0, FUN_005a9cf0, "thiscall(ptr[0x80]:{0x71:u8[0..1], 0x72:u16[0..0x24]}, ptr[8]:bytes) -> i32")
