#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Adds DAT_00603854 to the accumulator DAT_0063b840, then peels 0xa00 off it while it is
// positive, counting the peeled steps in DAT_00602a94. eax ends as the remainder and edx as the
// count; the flags are those of the last "cmp eax, 0" (CF = AF = OF = 0).
// FUNCTION: GP4 0x00434e54
void __cdecl FUN_00434e54(gp4::Regs* r) {
    uint32_t count = 0u;
    int32_t value = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0063b840) + GP4_GLOBAL(uint32_t, 0x00603854));
    while (value > 0) {
        count++;
        value -= 0xa00;
    }
    GP4_GLOBAL(int32_t, 0x0063b840) = value;
    GP4_GLOBAL(uint32_t, 0x00602a94) = count;

    r->eax = static_cast<uint32_t>(value);
    r->edx = count;
    r->eflags = (r->eflags & ~0x8d5u)
        | parityFlag(static_cast<uint32_t>(value)) | (value == 0 ? 0x40u : 0u)
        | ((static_cast<uint32_t>(value) >> 24) & 0x80u);
}
GP4_IMPL(0x00434e54, FUN_00434e54, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32; maxinsn=8000000")
