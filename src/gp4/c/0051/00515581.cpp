#include <gp4/gp4.h>
#include <gp4/flags.h>

// Resets the lap-time readout cursor and fills it with "no time" placeholders: selector [0x0088fe14] = 4,
// the cursor bytes 0x0088fd94..97 cleared, and six dword slots 0x008660dc..f0 set to (eax with al = '-').
// 0x0051446b then writes the six '-' characters, 0x0044334d refreshes the time digits, the event number
// [0x0088ff30] becomes 0x0e and the event dispatcher 0x00514152 runs. Afterwards the selector is -1, bit
// 0x04 of [esi + 0x89] is cleared (esi is the caller's record pointer, which the callees preserve) and
// [0x00954169] = 0xff. The registers are those the callee chain leaves; the flags are those of the final
// `and byte`.
// FUNCTION: GP4 0x00515581
void __cdecl FUN_00515581(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    GP4_GLOBAL(uint32_t, 0x0088fe14) = 4u;
    GP4_GLOBAL(uint8_t, 0x0088fd94) = 0u;
    GP4_GLOBAL(uint8_t, 0x0088fd95) = 0u;
    GP4_GLOBAL(uint8_t, 0x0088fd96) = 0u;
    GP4_GLOBAL(uint8_t, 0x0088fd97) = 0u;
    const uint32_t dash = (c.eax & 0xffffff00u) | 0x2du;
    GP4_GLOBAL(uint32_t, 0x008660dc) = dash;
    GP4_GLOBAL(uint32_t, 0x008660e0) = dash;
    GP4_GLOBAL(uint32_t, 0x008660e4) = dash;
    GP4_GLOBAL(uint32_t, 0x008660e8) = dash;
    GP4_GLOBAL(uint32_t, 0x008660ec) = dash;
    GP4_GLOBAL(uint32_t, 0x008660f0) = dash;

    gp4::call_regs(0x0051446b, c);
    gp4::call_regs(0x0044334d, c);
    GP4_GLOBAL(uint32_t, 0x0088ff30) = 0x0eu;
    gp4::call_regs(0x00514152, c);
    GP4_GLOBAL(uint32_t, 0x0088fe14) = 0xffffffffu;

    const uint8_t cleared = static_cast<uint8_t>(GP4_FIELD(uint8_t, c.esi, 0x89) & 0xfbu);
    GP4_FIELD(uint8_t, c.esi, 0x89) = cleared;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(cleared));
    GP4_GLOBAL(uint8_t, 0x00954169) = 0xffu;

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00515581, FUN_00515581, "regs(eax:u32, esi:ptr[0xa0]:bytes, edi:ptr[0x100]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=readout_00515187+glyph_0051d329+order_00418f8f+state_0044334d+player_0040f8d9+disp_005141b8+events_00514152")
