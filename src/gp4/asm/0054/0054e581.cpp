#include <gp4/gp4.h>
#include <gp4/flags.h>

// Compares the zero-terminated strings at esi and edi: eax = 0xffffffff when they are equal up to and
// including the terminator, 0 at the first differing byte (esi and edi are pushed and popped, so they
// come back unchanged). The flags are those of the last flag writer: `or al, al` (al = 0) when equal,
// the failing `cmp al, [edi]` otherwise.
// FUNCTION: GP4 0x0054e581
void __cdecl FUN_0054e581(gp4::Regs* r) {
    const uint8_t* a = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t* b = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->edi));

    for (;;) {
        const uint32_t ch = *a;
        if (ch != *b) {
            r->eax = 0u;
            r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(ch, *b));
            return;
        }
        ++a;
        ++b;
        if (ch == 0u) {
            r->eax = 0xffffffffu;
            r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(ch));
            return;
        }
    }
}
GP4_IMPL(0x0054e581, FUN_0054e581, "regs(esi:ptr[0x40]:u8{=0,0x41,0x42}, edi:ptr[0x40]:u8{=0,0x41,0x42}) -> (eax, cf, pf, af, zf, sf, of)")
