#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal / popal wrapper: removes the first eax bytes of the NUL-terminated string at esi by
// sliding the rest (terminator included) down over it with a forward `rep movsb`. The length comes
// from the strlen routine 0x0054d8d0. When the string is shorter than eax nothing moves and CF is
// set (the other flags stay those of `cmp len, eax`); otherwise the flags are those of the
// `sub (len + 1), eax` that sized the copy with CF cleared by `clc`. popal restores every
// register but not the flags.
// FUNCTION: GP4 0x0054d905
void __cdecl FUN_0054d905(gp4::Regs* r) {
    const uint32_t drop = r->eax;
    uint8_t* const text = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs c = *r;
    gp4::call_regs(0x0054d8d0, c);
    const uint32_t length = c.eax;

    uint32_t arith = gp4::flags::sub32(length, drop);   // cmp edx, ecx
    if (length < drop) {
        arith |= gp4::flags::CF;                         // stc
    } else {
        const uint32_t count = length + 1u - drop;
        arith = gp4::flags::sub32(length + 1u, drop) & ~gp4::flags::CF;   // inc eax; sub eax, ecx; clc
        uint8_t* dst = text;
        const uint8_t* src = text + drop;
        for (uint32_t i = count; i != 0u; --i) {
            *dst++ = *src++;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0054d905, FUN_0054d905, "regs(eax:u32[0..8], esi:ptr[0x40]:u8[0..3]) -> (cf, pf, af, zf, sf, of)")
