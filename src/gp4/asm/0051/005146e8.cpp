#include <gp4/gp4.h>
#include <gp4/flags.h>

// Skips blanks (space and tab) in the string at esi: loads bytes until one that is neither
// 0x20 nor 0x09 is found. al holds that byte (the rest of eax is untouched), esi points one past
// it and the flags are those of the final `cmp al, 9`.
// FUNCTION: GP4 0x005146e8
void __cdecl FUN_005146e8(gp4::Regs* r) {
    const uint8_t* p = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t ch;
    do {
        ch = *p++;
    } while (ch == 0x20u || ch == 0x09u);

    r->eax = (r->eax & 0xffffff00u) | ch;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p));
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(ch, 9u));
}
GP4_IMPL(0x005146e8, FUN_005146e8, "regs(eax:u32, esi:ptr[0x40]:u8[0..0x22]) -> (eax, esi, cf, pf, af, zf, sf, of)")
