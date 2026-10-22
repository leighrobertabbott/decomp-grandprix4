#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005c00e0 { uint8_t Unknown00[0x138]; void* Table138; uint8_t Unknown13c[0x4]; };
#pragma pack(pop)

// Forwards its stack argument to FUN_005cc340 on the word-table object held at +0x138.
// FUNCTION: GP4 0x005c00e0
uint8_t __fastcall FUN_005c00e0(Unknown_005c00e0* self, void* /*edx*/, uint32_t index) {
    return gp4::thiscall_<0x005cc340, uint8_t>(self->Table138, index);
}
GP4_IMPL(0x005c00e0, FUN_005c00e0, "thiscall(ptr[0x140]:{0x138:ptr[0x20008]:i16[-3..3]}, u32) -> u8")
