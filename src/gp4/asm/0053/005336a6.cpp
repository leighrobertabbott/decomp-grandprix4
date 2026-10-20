#include <gp4/gp4.h>
#include <gp4/flags.h>

// Releases every registered global-memory block listed in the 16 eight-byte entries at edi: for each
// entry whose first dword is nonzero, eax = the dword +4 and 0x0053367c runs (it keys on edx, not on eax,
// and leaves edi pointing at the matching entry of the table 0x00828020, or 0 when none matches).
// edi is then advanced by 8 and the counter ecx (starting at 16, restored by the final pop) decremented.
// esi is set to the table 0x00828020 (unused).
// FUNCTION: GP4 0x005336a6
void __cdecl FUN_005336a6(gp4::Regs* r) {
    const uint32_t saved_ecx = r->ecx;
    gp4::Regs c = *r;
    c.esi = 0x00828020u;
    c.ecx = 0x10u;
    for (;;) {
        const uint32_t used = GP4_FIELD(uint32_t, c.edi, 0);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(used, 0u));
        if (used != 0u) {
            c.eax = GP4_FIELD(uint32_t, c.edi, 4);
            gp4::call_regs(0x0053367c, c);
        }
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(c.edi, 8u));
        c.edi += 8u;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::dec32(c.ecx, c.eflags));
        c.ecx -= 1u;
        if (c.ecx == 0u) {
            break;
        }
    }
    c.ecx = saved_ecx;
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x005336a6, FUN_005336a6, "regs(edx:u32{=0}, edi:ptr[0x80]:bytes) -> (eax, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 GlobalHandle")
