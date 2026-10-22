#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005c8c40 { uint8_t Unknown00[0x4d]; uint8_t Value4d; uint8_t Value4e; uint8_t Unknown4f[0xb1]; };
#pragma pack(pop)

// Stores the low bytes of the two stack arguments at +0x4d and +0x4e of the object (`ret 8`).
// FUNCTION: GP4 0x005c8c40
void __fastcall FUN_005c8c40(Unknown_005c8c40* self, void* /*edx*/, uint8_t first, uint8_t second) {
    self->Value4d = first;
    self->Value4e = second;
}
GP4_IMPL(0x005c8c40, FUN_005c8c40, "thiscall(ptr[0x100]:bytes, i32, i32) -> void")
