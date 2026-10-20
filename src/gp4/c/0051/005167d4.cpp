#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stores min(max(([0x00675958] & 0x3f) - 1, 0), 0x15) in the dword 0x00a6987c (a 22-driver slot index).
// pushal/popal keep every register; the flags are those of `cmp ecx, 0x15` on the index after the lower
// clamp (the clamping `mov`s touch none).
// FUNCTION: GP4 0x005167d4
void __cdecl FUN_005167d4(gp4::Regs* r) {
    int32_t index = static_cast<int32_t>(GP4_GLOBAL(uint8_t, 0x00675958) & 0x3fu) - 1;
    if (index < 0) {
        index = 0;
    }
    const uint32_t arith = gp4::flags::sub32(static_cast<uint32_t>(index), 0x15u);
    if (index > 0x15) {
        index = 0x15;
    }
    GP4_GLOBAL(uint32_t, 0x00a6987c) = static_cast<uint32_t>(index);
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005167d4, FUN_005167d4, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
