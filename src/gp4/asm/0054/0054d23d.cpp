#include <gp4/gp4.h>
#include <gp4/flags.h>

// Reads two bytes b0, b1 from esi (lodsb, xchg ah/al, lodsb): edx = (b0 << 8) | b1. ebx = [0x833cc5],
// eax = [0x833cc9]; runs the two empty hooks 0x00531b35 and 0x00531b34, then adds ebp to [0x833cc5]
// (flags of that add) and sets the byte [0x833f74] to 0xff.
// FUNCTION: GP4 0x0054d23d
void __cdecl FUN_0054d23d(gp4::Regs* r) {
    const uint8_t* src = reinterpret_cast<const uint8_t*>(r->esi);
    const uint32_t pair = (static_cast<uint32_t>(src[0]) << 8) | src[1];

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.esi = r->esi + 2u;
    c.edx = pair;
    c.ebx = GP4_GLOBAL(uint32_t, 0x00833cc5);
    c.eax = GP4_GLOBAL(uint32_t, 0x00833cc9);
    gp4::call_regs(0x00531b35, c);
    gp4::call_regs(0x00531b34, c);

    const uint32_t before = GP4_GLOBAL(uint32_t, 0x00833cc5);
    GP4_GLOBAL(uint32_t, 0x00833cc5) = before + c.ebp;
    GP4_GLOBAL(uint8_t, 0x00833f74) = 0xffu;

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(before, c.ebp));
}
GP4_IMPL(0x0054d23d, FUN_0054d23d, "regs(esi:ptr[0x10]:bytes, ebp:u32) -> (eax, ebx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
