#include <gp4/gp4.h>
#include <gp4/flags.h>

// Classifies the top three bits of the byte 0x004015dc returns in al: v = (al >> 5) is kept for 0 and 1,
// 2..4 become 2 and 5..7 become 3. The flags are those of the last compare (`cmp eax, 2` for v < 2,
// otherwise `cmp eax, 4`).
// FUNCTION: GP4 0x0052572e
void __cdecl FUN_0052572e(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x004015dc, c);

    const uint32_t v = (c.eax & 0xffu) >> 5;
    uint32_t result = v;
    uint32_t flags;
    if (static_cast<int32_t>(v) < 2) {
        flags = gp4::flags::sub32(v, 2u);
    } else {
        flags = gp4::flags::sub32(v, 4u);
        result = (static_cast<int32_t>(v) <= 4) ? 2u : 3u;
    }

    r->eax = result;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0052572e, FUN_0052572e, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0062d1b4")
