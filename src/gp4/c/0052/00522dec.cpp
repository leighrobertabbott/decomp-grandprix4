#include <gp4/gp4.h>
#include <gp4/flags.h>

// Scans the 22 per-driver flag bytes at 0x007c2a48 for the first one with both bit 7 and bit 6 set and
// returns it in al (al = 1 when none qualifies); the rest of eax is preserved. ecx and esi are pushed
// and popped. Flags: those of the `test byte [esi], 0x40` that found the byte, otherwise those of the
// last `inc esi` (22nd hop, CF clear because a test always precedes it).
// FUNCTION: GP4 0x00522dec
void __cdecl FUN_00522dec(gp4::Regs* r) {
    const uint8_t* driver_flags = GP4_ARRAY(uint8_t, 0x007c2a48);
    uint32_t al = 1u;
    uint32_t flags = 0u;
    for (uint32_t i = 0; i < 0x16u; ++i) {
        const uint32_t b = driver_flags[i];
        flags = gp4::flags::logic8(b & 0x80u);
        if ((b & 0x80u) != 0u) {
            flags = gp4::flags::logic8(b & 0x40u);
            if ((b & 0x40u) != 0u) {
                al = b;
                break;
            }
        }
        flags = gp4::flags::inc32(0x007c2a48u + i, flags);
    }
    r->eax = (r->eax & 0xffffff00u) | al;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00522dec, FUN_00522dec, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=drv_005117d3")
