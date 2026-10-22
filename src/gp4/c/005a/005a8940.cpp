#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005a8940 { uint8_t Unknown00[0x3c]; uint8_t Flag3c; uint8_t Flag3d; };
#pragma pack(pop)

// Hangs forever (`jmp $`) when the byte at +0x3c is 1; otherwise clears the byte at +0x3d and returns 0.
// FUNCTION: GP4 0x005a8940
int __fastcall FUN_005a8940(Unknown_005a8940* self, void* /*edx*/) {
    if (self->Flag3c == 1) {
        for (;;) {
        }
    }
    self->Flag3d = 0;
    return 0;
}
GP4_IMPL(0x005a8940, FUN_005a8940, "thiscall(ptr[0x48]:{0x3c:u8{=0,2,3,0x80,0xff}, 0x3d:u8[0..255]}) -> i32")
