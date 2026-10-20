#include <gp4/gp4.h>
#include <gp4/flags.h>

// R3 stream handler, active only in mode 1 ([0x86857c]); in any other mode it just runs `add esi, 0`
// (esi unchanged, flags of that add). In mode 1 the record edi is remembered in [0x866f04] when cl is
// 7 (flags of `cmp cl, 7`); otherwise 0x00559636 runs first (it leaves ebx at its source end and sets
// the flags) and edi goes to [0x866f00].
// FUNCTION: GP4 0x00556f9f
void __cdecl FUN_00556f9f(gp4::Regs* r) {
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 1u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(r->esi, 0u));
        return;
    }
    const uint32_t kind = r->ecx & 0xffu;
    if (kind == 7u) {
        GP4_GLOBAL(uint32_t, 0x00866f04) = r->edi;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(kind, 7u));
        return;
    }
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00559636, c);
    GP4_GLOBAL(uint32_t, 0x00866f00) = r->edi;
    r->ebx = c.ebx;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00556f9f, FUN_00556f9f, "regs(ecx:u32{7,7,0,1,0x107,0xff}, esi:u32, edi:u32) -> (ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_0086857c+words_00559636")
