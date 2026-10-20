#include <gp4/gp4.h>
#include <gp4/flags.h>

// Skips a line: loads al = *esi++ until al == '\n' (0x0a). On exit al = '\n', esi points past it
// and the flags are those of the final `cmp al, 0x0a` (equal).
// FUNCTION: GP4 0x005146f2
void __cdecl FUN_005146f2(gp4::Regs* r) {
    const uint8_t* s = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t al;
    do {
        al = *s++;
    } while (al != 0x0au);

    r->eax = (r->eax & 0xffffff00u) | al;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(s));
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(al, 0x0au));
}
GP4_IMPL(0x005146f2, FUN_005146f2, "regs(eax:u32, esi:ptr[0x40]:u8{0x0a,0x20,0x41,0x00}) -> (eax, esi, cf, pf, af, zf, sf, of)")
