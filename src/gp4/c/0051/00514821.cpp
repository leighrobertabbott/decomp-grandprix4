#include <gp4/gp4.h>
#include <gp4/flags.h>

// Parses one record of the HUD/option text at esi into the byte buffer at edi: the keyword index of the
// word list at 0x008915ac ("lcdp_CAR_NUMBER", ...; 0x005147f6) is stored first, then six decimal
// numbers (blanks and commas skipped by 0x005146e8, value parsed by 0x00514748), one byte each. ebx and
// ecx are saved and restored; eax is the last number (the full parsed value), esi stops on the byte
// after it and edi after the seven stored bytes. The flags are those 0x00514748 leaves (`loop` keeps them).
// FUNCTION: GP4 0x00514821
void __cdecl FUN_00514821(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.ebx = 0x008915acu;
    gp4::call_regs(0x005147f6, c);
    *(uint8_t*)(uintptr_t)c.edi = (uint8_t)c.eax;
    c.edi += 1u;
    uint32_t left = 6u;
    do {
        c.ecx = left;
        do {
            gp4::call_regs(0x005146e8, c);
        } while ((c.eax & 0xffu) == 0x2cu);
        c.esi -= 1u;
        gp4::call_regs(0x00514748, c);
        *(uint8_t*)(uintptr_t)c.edi = (uint8_t)c.eax;
        c.edi += 1u;
        left -= 1u;
    } while (left != 0u);
    r->eax = c.eax;
    r->esi = c.esi;
    r->edi = c.edi;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00514821, FUN_00514821, "regs(eax:u32, esi:ptr[0x100]:u8{=0x30,0x31,0x35,0x39,0x41,0x7a,0x80,0xff,0x2d,0x2b,0x2c,0x0d,0x0a,0x20,0x09,0x6c,0x63,0x64,0x70}, edi:ptr[0x10]:bytes) -> (eax, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
