#include <gp4/gp4.h>
#include <gp4/flags.h>

// Dword at 4 * index + base (kept as a helper so the table bases stay separate globals).
static __declspec(noinline) uint32_t TableAt(const uint32_t* base, uint32_t index) {
    return base[index];
}

// Distance between the position of object [0x679bd8] (record pointer 0x6f2f6c[i]) and the record
// [0x679be0]. Each record pointer at or above the pit start [0x891500] is replaced by its dword
// +0x44. The difference (b - a) is negated when its magnitude reaches the half-lap [0x866ed8]
// (signed), and its high half (sar 16) goes to the words 0x875e90/92; 0x875e94/96 are cleared and
// the dwords 0x6f336c[i], 0x6f32ec[i] are copied to 0x6f3ad0, 0x6f3ad4. pushal/popal keep every
// register; the flags are those of the final `sar edx, 16`.
// FUNCTION: GP4 0x00526fae
void __cdecl FUN_00526fae(gp4::Regs* r) {
    const uint32_t index = GP4_GLOBAL(uint32_t, 0x00679bd8);
    uint32_t a = TableAt(GP4_ARRAY(uint32_t, 0x006f2f6c), index);
    uint32_t b = GP4_GLOBAL(uint32_t, 0x00679be0);
    if (a >= GP4_GLOBAL(uint32_t, 0x00891500)) {
        a = GP4_FIELD(uint32_t, a, 0x44);
    }
    if (b >= GP4_GLOBAL(uint32_t, 0x00891500)) {
        b = GP4_FIELD(uint32_t, b, 0x44);
    }
    uint32_t diff = b - a;
    uint32_t mag = diff;
    if (static_cast<int32_t>(mag) < 0) {
        mag = 0u - mag;
    }
    if (static_cast<int32_t>(mag) >= static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00866ed8))) {
        diff = 0u - diff;
    }
    const int32_t shifted = static_cast<int32_t>(diff) >> 16;
    const uint16_t half = static_cast<uint16_t>(shifted);
    GP4_GLOBAL(uint16_t, 0x00875e92) = half;
    GP4_GLOBAL(uint16_t, 0x00875e90) = half;
    GP4_GLOBAL(uint16_t, 0x00875e96) = 0;
    GP4_GLOBAL(uint16_t, 0x00875e94) = 0;
    GP4_GLOBAL(uint32_t, 0x006f3ad0) = TableAt(GP4_ARRAY(uint32_t, 0x006f336c), index);
    GP4_GLOBAL(uint32_t, 0x006f3ad4) = TableAt(GP4_ARRAY(uint32_t, 0x006f32ec), index);

    // sar by 16: CF = bit 15 of the operand, AF = OF = 0, SF/ZF/PF from the result.
    const uint32_t flags = gp4::flags::logic32(static_cast<uint32_t>(shifted)) | ((diff >> 15) & 1u);
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00526fae, FUN_00526fae, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ptrtab_00524fa4+ring_00525205+ptrs_00526fae")
