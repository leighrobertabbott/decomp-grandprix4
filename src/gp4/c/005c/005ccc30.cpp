#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005ccc30 { uint8_t Unknown00[0x14]; int32_t Value14; int32_t Value18; uint8_t Flag1c; uint8_t Unknown1d[0xe3]; };
#pragma pack(pop)

// Sets the byte at +0x1c to 1 and copies the globals 0x009ed5bc and 0x009ed5b8 into the dwords
// at +0x14 and +0x18.
// FUNCTION: GP4 0x005ccc30
void __fastcall FUN_005ccc30(Unknown_005ccc30* self, void* /*edx*/) {
    self->Flag1c = 1;
    self->Value14 = GP4_GLOBAL(int32_t, 0x009ed5bc);
    self->Value18 = GP4_GLOBAL(int32_t, 0x009ed5b8);
}
GP4_IMPL(0x005ccc30, FUN_005ccc30, "thiscall(ptr[0x100]:bytes) -> void; globals=fuzz:i32")
