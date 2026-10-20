#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal / popal around an in-place copy of the NUL-terminated string at esi that drops every
// space (0x20); the terminator is copied too. All registers are restored by popal. The loop only
// ends after `or al, al` saw the terminator, and popal does not restore flags, so the flags that
// escape are those of `or al, 0`-style logic on zero: ZF = PF = 1, the others clear.
// FUNCTION: GP4 0x0054d95a
void __cdecl FUN_0054d95a(gp4::Regs* r) {
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t ch;
    do {
        do {
            ch = *src++;
        } while (ch == 0x20u);
        *dst++ = ch;
    } while (ch != 0u);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(ch));
}
GP4_IMPL(0x0054d95a, FUN_0054d95a, "regs(esi:ptr[0x200]:u8[0..0x20]) -> (cf, pf, af, zf, sf, of)")
