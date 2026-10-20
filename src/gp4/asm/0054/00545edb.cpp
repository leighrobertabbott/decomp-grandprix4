#include <gp4/gp4.h>
#include <gp4/flags.h>

// Writes a "first - second" pair of texts for the driver slot k = eax: the text script of id k + 0xba6
// through the interpreter 0x0054d798, a '-' byte at the output cursor edi (which the interpreter
// advanced), then the script of id k + 0xbb1. esi and eax are restored (push/pop); the other registers and
// the flags are what the second call leaves.
// FUNCTION: GP4 0x00545edb
void __cdecl FUN_00545edb(gp4::Regs* r) {
    const uint32_t slot = r->eax;
    gp4::Regs c = *r;
    c.esi = slot + 0xba6u;
    c.eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(slot, 0xba6u));
    gp4::call_regs(0x0054d798, c);

    GP4_FIELD(uint8_t, c.edi, 0) = 0x2du;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::inc32(c.edi, c.eflags));
    c.edi = c.edi + 1u;
    c.esi = slot + 0xbb1u;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(slot, 0xbb1u));
    gp4::call_regs(0x0054d798, c);

    c.esi = r->esi;
    c.eax = r->eax;
    *r = c;
}
GP4_IMPL(0x00545edb, FUN_00545edb, "regs(eax:u32[0..10], ebx:u32, ecx:u32, edx:u32, edi:ptr[0x100]:bytes, ebp:u32, esi:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798")
