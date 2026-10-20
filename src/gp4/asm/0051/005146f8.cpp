#include <gp4/gp4.h>
#include <gp4/flags.h>

// Scans the string at esi for a ':' (0x3a) while walking ebx in step: each iteration loads
// al = *esi++ and ah = *ebx++ (the original's `cmp al, ah; je` goes to the next instruction on
// both paths). On exit al = ':' and ah is the last byte read through ebx; the flags are those of
// the final `cmp al, 0x3a` (equal).
// FUNCTION: GP4 0x005146f8
void __cdecl FUN_005146f8(gp4::Regs* r) {
    const uint8_t* s = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t* t = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t al;
    uint8_t ah;
    do {
        al = *s++;
        ah = *t++;
    } while (al != 0x3au);

    r->eax = (r->eax & 0xffff0000u) | (static_cast<uint32_t>(ah) << 8) | al;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(s));
    r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(t));
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(al, 0x3au));
}
GP4_IMPL(0x005146f8, FUN_005146f8, "regs(eax:u32, ebx:ptr[0x100]:bytes, esi:ptr[0x40]:u8[0x38..0x3c]) -> (eax, ebx, esi, cf, pf, af, zf, sf, of)")
