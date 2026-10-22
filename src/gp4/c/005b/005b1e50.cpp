#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b1e50 { uint8_t Unknown00[4]; uint16_t Word04; uint8_t Unknown06[0xfa]; };
#pragma pack(pop)

// Stores the low word of its stack argument at +4 of the object and returns 0.
// FUNCTION: GP4 0x005b1e50
int __fastcall FUN_005b1e50(Unknown_005b1e50* self, void* /*edx*/, uint16_t value) {
    self->Word04 = value;
    return 0;
}
GP4_IMPL(0x005b1e50, FUN_005b1e50, "thiscall(ptr[0x100]:bytes, i32) -> i32")
