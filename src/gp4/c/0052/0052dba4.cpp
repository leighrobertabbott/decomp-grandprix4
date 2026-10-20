#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stores 0x0f to the byte 0x00824649, then runs the linked-list flag setter 0x0053e612 with
// esi = 0x0084bc2a (the list head) and eax = 4. That routine restores every register, so eax = 4 and
// esi = 0x0084bc2a remain; the flags are whatever it leaves.
// FUNCTION: GP4 0x0052dba4
void __cdecl FUN_0052dba4(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x00824649) = 0x0fu;
    gp4::Regs c = *r;
    c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x0084bc2a)));
    c.eax = 4u;
    gp4::call_regs(0x0053e612, c);
    r->esi = c.esi;
    r->eax = c.eax;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0052dba4, FUN_0052dba4, "regs() -> (eax, esi, cf, pf, af, zf, sf, of); globals=layout=list_0052dba4")
