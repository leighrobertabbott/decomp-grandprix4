#include <gp4/gp4.h>
#include <gp4/flags.h>

// Flags of "sar reg, 5" (count > 1): CF = bit 4 of the operand, OF = AF = 0, ZF/SF/PF of the result.
static inline uint32_t sar5Flags(uint32_t value) {
    const uint32_t result = static_cast<uint32_t>(static_cast<int32_t>(value) >> 5);
    return gp4::flags::szp(result, 32) | ((value >> 4) & 1u);
}

// Snapshot of the record at [0x00a69878] (esi) into the 0x00a6987c.. globals:
//   [0x00a6987c] = eax >> 5 and [0x00a69884] = edx >> 5 (arithmetic) of 0x004045d3 (dwords +0, +4),
//   [0x00a69880] = edx >> 5 of 0x00402f8a (dword +8), [0x00a69888] = eax of 0x00408fa2.
// pushal/popal wrapper: every GPR is restored; the flags are those 0x00408fa2 leaves.
// FUNCTION: GP4 0x0051f924
void __cdecl FUN_0051f924(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.esi = GP4_GLOBAL(uint32_t, 0x00a69878);

    gp4::call_regs(0x004045d3, c);
    GP4_GLOBAL(int32_t, 0x00a6987c) = static_cast<int32_t>(c.eax) >> 5;
    c.eflags = gp4::flags::merge(c.eflags, sar5Flags(c.eax));
    GP4_GLOBAL(int32_t, 0x00a69884) = static_cast<int32_t>(c.edx) >> 5;
    c.eflags = gp4::flags::merge(c.eflags, sar5Flags(c.edx));

    gp4::call_regs(0x00402f8a, c);
    GP4_GLOBAL(int32_t, 0x00a69880) = static_cast<int32_t>(c.edx) >> 5;
    c.eflags = gp4::flags::merge(c.eflags, sar5Flags(c.edx));

    gp4::call_regs(0x00408fa2, c);
    GP4_GLOBAL(uint32_t, 0x00a69888) = c.eax;

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051f924, FUN_0051f924, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ptr_00a69878")
