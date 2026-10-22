#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005ab350 { uint8_t Unknown00[0x70]; uint16_t Word70; uint8_t Unknown72[0x8e]; };
#pragma pack(pop)

// Stores the low word of its stack argument at +0x70 of the object and returns 0.
// FUNCTION: GP4 0x005ab350
int __fastcall FUN_005ab350(Unknown_005ab350* self, void* /*edx*/, uint16_t value) {
    self->Word70 = value;
    return 0;
}
GP4_IMPL(0x005ab350, FUN_005ab350, "thiscall(ptr[0x100]:bytes, i32) -> i32")
