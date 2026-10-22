#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005cd400 { uint8_t Unknown00[0x34]; int32_t Value34; int32_t Value38; uint8_t Unknown3c[0xc4]; };
#pragma pack(pop)

// Stores the two stack arguments at +0x34 and +0x38 of the object (`ret 8`).
// FUNCTION: GP4 0x005cd400
void __fastcall FUN_005cd400(Unknown_005cd400* self, void* /*edx*/, int32_t first, int32_t second) {
    self->Value34 = first;
    self->Value38 = second;
}
GP4_IMPL(0x005cd400, FUN_005cd400, "thiscall(ptr[0x100]:bytes, i32, i32) -> void")
