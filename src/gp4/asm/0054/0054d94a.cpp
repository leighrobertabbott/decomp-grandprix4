#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal / popal around an in-place translation of the NUL-terminated string at esi: every byte
// goes through the 256-byte table lookup 0x0054d755 and is stored back (the loop stops after a
// translated 0, which is stored too). popal restores every register and not the flags, so the
// flags that escape are those of the final `or al, al` on zero.
// FUNCTION: GP4 0x0054d94a
void __cdecl FUN_0054d94a(gp4::Regs* r) {
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t ch;
    do {
        gp4::Regs c = {};
        c.eax = *src++;
        gp4::call_regs(0x0054d755, c);
        ch = static_cast<uint8_t>(c.eax);
        *dst++ = ch;
    } while (ch != 0u);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(ch));
}
GP4_IMPL(0x0054d94a, FUN_0054d94a, "regs(esi:ptr[0x40]:u8{0x00,0x41,0x61,0x20}) -> (cf, pf, af, zf, sf, of); globals=layout=lower_00833e18")
