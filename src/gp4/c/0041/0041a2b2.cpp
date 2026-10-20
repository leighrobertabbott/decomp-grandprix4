#include <gp4/gp4.h>
#include <gp4/flags.h>

// For the player car [0x66d820] and, when byte [0x7c2e6c] is set, the second car [0x66d82c]: calls the
// per-car hook (0x0051f1ee for the first, 0x0051f1f6 for the second) when bit 3 of byte +0xcf is set.
// esi is left holding the last car pointer loaded; the flags are those of the last flag writer (the
// hook's on a call, otherwise the last test / cmp / or). Each hook is entered with the flags of the
// `test byte [esi+0xcf], 8` before it.
// FUNCTION: GP4 0x0041a2b2
void __cdecl FUN_0041a2b2(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    uint32_t esi = GP4_GLOBAL(uint32_t, 0x0066d820);
    uint32_t flags = gp4::flags::logic32(esi);
    if (esi != 0u) {
        const uint8_t status = GP4_FIELD(uint8_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(esi)), 0xcf);
        flags = gp4::flags::logic8(status & 8u);
        if ((status & 8u) != 0u) {
            c.esi = esi;
            c.eflags = gp4::flags::merge(c.eflags, flags);
            gp4::call_regs(0x0051f1ee, c);
            flags = c.eflags;
        }
    }

    const uint8_t second = GP4_GLOBAL(uint8_t, 0x007c2e6c);
    flags = gp4::flags::logic8(second);
    if (second != 0u) {
        esi = GP4_GLOBAL(uint32_t, 0x0066d82c);
        flags = gp4::flags::logic32(esi);
        if (esi != 0u) {
            const uint8_t status = GP4_FIELD(uint8_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(esi)), 0xcf);
            flags = gp4::flags::logic8(status & 8u);
            if ((status & 8u) != 0u) {
                c.esi = esi;
                c.eflags = gp4::flags::merge(c.eflags, flags);
                gp4::call_regs(0x0051f1f6, c);
                flags = c.eflags;
            }
        }
    }

    r->esi = esi;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0041a2b2, FUN_0041a2b2, "regs() -> (esi, cf, pf, af, zf, sf, of); globals=layout=state_0041104f+cf_0041a2b2")
