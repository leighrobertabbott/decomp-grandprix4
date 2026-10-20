#include <gp4/gp4.h>
#include <gp4/flags.h>

// Clears the two flag dwords 0x00a6987c and 0x00a69880, then, when bit 5 of byte +0x82 of the
// selected car record [0x00679d30] is set, sets [0x00a6987c] = 1 if the car byte +0x85 is 5 and
// [0x00a69880] = 1 if it is 4. pushal/popal wrapper: every GPR is restored, the flags are those of
// the last flag writer (popal does not restore them): "test byte [esi+0x82],0x20" when the bit is
// clear, otherwise "cmp eax,4".
// FUNCTION: GP4 0x005165cd
void __cdecl FUN_005165cd(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x00a6987c) = 0u;
    GP4_GLOBAL(uint32_t, 0x00a69880) = 0u;
    const uint8_t* car = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00679d30)));
    const uint32_t gate = car[0x82] & 0x20u;
    uint32_t arith = gp4::flags::logic8(gate);
    if (gate != 0u) {
        const uint32_t kind = car[0x85];
        if (kind == 5u) {
            GP4_GLOBAL(uint32_t, 0x00a6987c) = 1u;
        }
        arith = gp4::flags::sub32(kind, 4u);
        if (kind == 4u) {
            GP4_GLOBAL(uint32_t, 0x00a69880) = 1u;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005165cd, FUN_005165cd, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=carbg_0066fef0+state_0044334d+kind_005165cd")
