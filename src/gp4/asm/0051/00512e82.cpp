#include <gp4/gp4.h>
#include <gp4/flags.h>

// Reads the circular byte buffer [0x007a8e28, 0x007a8e2c) starting at offset [0x0088f604] into the
// buffer at edi, one byte per position of the buffer (ecx = end - start bytes, wrapping at the
// end). Leaves ecx = 0, esi = the read position after the last byte and edi advanced by the
// count; the flags are those of the last `cmp esi, end` (the wrap's `mov` does not change them).
// FUNCTION: GP4 0x00512e82
void __cdecl FUN_00512e82(gp4::Regs* r) {
    const uint32_t start = GP4_GLOBAL(uint32_t, 0x007a8e28);
    const uint32_t end = GP4_GLOBAL(uint32_t, 0x007a8e2c);
    uint32_t source = start + GP4_GLOBAL(uint32_t, 0x0088f604);
    uint32_t count = end - start;
    uint32_t target = r->edi;
    uint32_t arith = 0u;
    do {
        GP4_FIELD(uint8_t, target, 0) = GP4_FIELD(uint8_t, source, 0);
        ++target;
        ++source;
        arith = gp4::flags::sub32(source, end);
        if (source >= end) {
            source = start;
        }
        --count;
    } while (count != 0u);
    r->ecx = 0u;
    r->esi = source;
    r->edi = target;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00512e82, FUN_00512e82, "regs(edi:ptr[0x1128]:bytes) -> (ecx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=ring_00512e82")
