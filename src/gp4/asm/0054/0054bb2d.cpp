#include <gp4/gp4.h>
#include <gp4/flags.h>

// strlen: eax = number of bytes before the first zero at esi (esi is restored). The flags are
// those of the final `cmp byte [esi], 0` that found the terminator.
// FUNCTION: GP4 0x0054bb2d
void __cdecl FUN_0054bb2d(gp4::Regs* r) {
    const uint8_t* text = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t length = 0u;
    while (text[length] != 0u) {
        ++length;
    }
    r->eax = length;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(0u, 0u));
}
GP4_IMPL(0x0054bb2d, FUN_0054bb2d, "regs(esi:ptr[0x40]:u8{0x00,0x41,0x42,0x43}) -> (eax, cf, pf, af, zf, sf, of)")
