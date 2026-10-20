#include <gp4/gp4.h>
#include <gp4/flags.h>

// Parses one record of the HUD/option text at esi into the byte buffer chosen by the keyword: the keyword
// index in the word list at 0x0089167c ("hlight_TYRE", ...; 0x005147f6) selects the destination pointer
// in the table at 0x00891708, then four decimal numbers (blanks and commas skipped by 0x005146e8, value
// parsed by 0x00514748) are stored one byte each. ebx, ecx and edi are saved and restored; eax is the
// last number (the full parsed value), esi stops on the byte after it. The flags are those 0x00514748
// leaves (`loop` keeps them).
// FUNCTION: GP4 0x00514848
void __cdecl FUN_00514848(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.ebx = 0x0089167cu;
    gp4::call_regs(0x005147f6, c);
    uint8_t* dst = (uint8_t*)(uintptr_t)GP4_ARRAY(uint32_t, 0x00891708)[c.eax];
    uint32_t left = 4u;
    do {
        c.ecx = left;
        do {
            gp4::call_regs(0x005146e8, c);
        } while ((c.eax & 0xffu) == 0x2cu);
        c.esi -= 1u;
        gp4::call_regs(0x00514748, c);
        *dst = (uint8_t)c.eax;
        dst += 1;
        left -= 1u;
    } while (left != 0u);
    r->eax = c.eax;
    r->esi = c.esi;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00514848, FUN_00514848, "regs(eax:u32, esi:ptr[0x100]:layout=text_00514877+hrec_00514848) -> (eax, esi, cf, pf, af, zf, sf, of)")
