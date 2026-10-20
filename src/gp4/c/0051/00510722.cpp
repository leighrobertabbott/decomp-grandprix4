#include <gp4/gp4.h>
#include <gp4/x87.h>
#include <gp4/flags.h>

// pushal wrapper: the selected car record [0x679d30] gives (through 0x00409ab1, esi = record) a 0x60-byte
// table entry; the double at +0x18 is rounded with fistp into [0x6f2bf8], whose value >> 8 goes to
// [0xa69884], and [0x6f2c08] is copied to [0xa69888]. popal restores every GPR; the flags are those of
// the final `sar eax, 8`.
// FUNCTION: GP4 0x00510722
void __cdecl FUN_00510722(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.esi = GP4_GLOBAL(uint32_t, 0x00679d30);
    gp4::call_regs(0x00409ab1, c);

    uint8_t* entry = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.eax));
    GP4_GLOBAL(int32_t, 0x006f2bf8) = gp4::x87::fistp32(GP4_FIELD(double, entry, 0x18));

    const int32_t rounded = GP4_GLOBAL(int32_t, 0x006f2bf8);
    const int32_t shifted = rounded >> 8;
    GP4_GLOBAL(int32_t, 0x00a69884) = shifted;
    GP4_GLOBAL(uint32_t, 0x00a69888) = GP4_GLOBAL(uint32_t, 0x006f2c08);

    // sar by 8: CF = bit 7 of the value, OF = AF = 0, SF/ZF/PF from the result.
    const uint32_t flags = gp4::flags::logic32(static_cast<uint32_t>(shifted)) |
                           ((static_cast<uint32_t>(rounded) >> 7) & 1u);
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00510722, FUN_00510722, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ptr_00679d30")
