#include <gp4/gp4.h>
#include <gp4/flags.h>

// Key-bound mode switch: 0x0052dbbc pops a key word from the key queue and looks it up (CF set: nothing
// to do, eax and the flags are whatever it left). On a hit the key word is stored at [0x007ab564], the
// item chain starting at the list head 0x0084bc2a is walked by 0x0053e4db with eax = 2, the byte
// [0x0082fd45] becomes 1, the chain flag setter 0x0053e612 runs with eax = 7 on the same head and the mode
// byte [0x00824649] becomes 0x0d. Both chain routines restore every register, so eax = 7 and
// esi = 0x0084bc2a remain; the flags are those of 0x0053e612 (the zero-link compare).
// FUNCTION: GP4 0x0052db4c
void __cdecl FUN_0052db4c(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    gp4::call_regs(0x0052dbbc, c);
    if ((c.eflags & gp4::CF) == 0u) {
        GP4_GLOBAL(uint16_t, 0x007ab564) = static_cast<uint16_t>(c.eax);
        c.eax = 2u;
        c.esi = 0x0084bc2au;
        gp4::call_regs(0x0053e4db, c);
        GP4_GLOBAL(uint8_t, 0x0082fd45) = 1u;
        c.esi = 0x0084bc2au;
        c.eax = 7u;
        gp4::call_regs(0x0053e612, c);
        GP4_GLOBAL(uint8_t, 0x00824649) = 0x0du;
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0052db4c, FUN_0052db4c, "regs(eax:u32) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keyq_008245c0+script_0054d798+list_0052dba4")
