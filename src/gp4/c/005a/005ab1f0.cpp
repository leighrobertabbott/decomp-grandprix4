#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005ab1f0 { uint8_t Unknown00[0x84]; uint8_t Flag84; uint8_t Unknown85[0x27]; int32_t ValueAc; uint8_t UnknownB0[0x50]; };
#pragma pack(pop)

// Returns the dword at +0xac when the byte flag at +0x84 is set, else 0.
// FUNCTION: GP4 0x005ab1f0
int32_t __fastcall FUN_005ab1f0(Unknown_005ab1f0* self, void* /*edx*/) {
    if (self->Flag84 == 0) {
        return 0;
    }
    return self->ValueAc;
}
GP4_IMPL(0x005ab1f0, FUN_005ab1f0, "thiscall(ptr[0x100]:{0x84:u8{0,1,0x80}, 0xac:u32}) -> i32")
