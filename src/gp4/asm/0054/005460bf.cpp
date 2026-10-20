#include <gp4/gp4.h>
#include <gp4/flags.h>

// Twin of 0x005460b3 that first consumes a big-endian 16-bit parameter: `inc esi`, the word at esi is
// byte-swapped (xchg al, ah) and passed to 0x005462cb (scales it into the pair at 0x831560), the cursor
// moves past the word, then 0x005461b9 (draws the string at esi) and 0x00546154 (skips the cursor over
// bytes >= 0x14). Registers and flags are whatever the last call leaves.
// FUNCTION: GP4 0x005460bf
void __cdecl FUN_005460bf(gp4::Regs* r) {
    gp4::Regs c = *r;
    const uint32_t before = r->esi;
    c.esi = before + 1u;
    c.eflags = gp4::flags::merge(r->eflags, gp4::flags::inc32(before, r->eflags));
    const uint32_t raw = GP4_FIELD(uint16_t, c.esi, 0);
    c.eax = ((raw & 0xffu) << 8) | (raw >> 8);
    const uint32_t args[1] = {c.eax};
    gp4::call_regs_stdcall(0x005462cb, c, args);
    const uint32_t cursor = c.esi;
    c.esi = cursor + 2u;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(cursor, 2u));
    gp4::call_regs(0x005461b9, c);
    gp4::call_regs(0x00546154, c);
    *r = c;
}
GP4_IMPL(0x005460bf, FUN_005460bf, "regs(esi:ptr[0x200]:bytes) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 GetTextExtentPoint32A")
