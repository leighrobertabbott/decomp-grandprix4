#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00481980 {
    uint8_t Unknown00[9];
    uint8_t Flag09;
    uint8_t Flag0a;
};
#pragma pack(pop)

// Cycles the two flag bytes at +9 / +0xa of the object, but only while the dwords 0x0066d820 and
// 0x00679d30 are equal: (1,1) -> (+9 stays 1, +0xa = 0); (+9 = 1, +0xa = 0) -> (+9 = 0, +0xa = 1);
// anything else -> (1,1). Then copies the byte at +9 into the dword 0x00645954.
// FUNCTION: GP4 0x00481980
void __fastcall FUN_00481980(Unknown_00481980* self) {
    if (GP4_GLOBAL(uint32_t, 0x0066d820) == GP4_GLOBAL(uint32_t, 0x00679d30)) {
        if (self->Flag0a == 1 && self->Flag09 == 1) {
            self->Flag0a = 0;
        } else if (self->Flag0a == 0 && self->Flag09 == 1) {
            self->Flag0a = 1;
            self->Flag09 = 0;
        } else {
            self->Flag09 = 1;
            self->Flag0a = 1;
        }
        GP4_GLOBAL(uint32_t, 0x00645954) = self->Flag09;
    }
}
GP4_IMPL(0x00481980, FUN_00481980, "thiscall(ptr[0x10]:{9:u8{0,1,2}, 0xa:u8{0,1,2}}) -> void; globals=fuzz:i32; gfields={0x0066d820:u32{=0,1}, 0x00679d30:u32{=0,1}}")
