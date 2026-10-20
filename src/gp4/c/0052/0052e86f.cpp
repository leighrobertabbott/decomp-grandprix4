#include <gp4/gp4.h>
#include <gp4/flags.h>

// Scans the NUL-terminated path buffer at 0x0081e7f5. edi ends on the terminator, ebx on the last '\'
// of the string (or on its start when there is none) and ecx counts 1 + the number of '\' for a
// non-empty string (0 for an empty one). Every exit follows a `cmp byte [edi], 0` of a zero byte, so
// the flags are those of 0 - 0 (ZF and PF set).
// FUNCTION: GP4 0x0052e86f
void __cdecl FUN_0052e86f(gp4::Regs* r) {
    uint32_t edi = 0x0081e7f5u;
    uint32_t ebx = edi;
    uint32_t ecx = 0u;
    if (GP4_FIELD(uint8_t, edi, 0) != 0u) {
        ecx = 1u;
        for (;;) {
            const uint8_t c = GP4_FIELD(uint8_t, edi, 0);
            if (c == 0u) {
                break;
            }
            if (c == 0x5cu) {
                ebx = edi;
                ++ecx;
            }
            ++edi;
        }
    }
    r->edi = edi;
    r->ebx = ebx;
    r->ecx = ecx;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(0u, 0u));
}
GP4_IMPL(0x0052e86f, FUN_0052e86f, "regs() -> (ebx, ecx, edi, cf, pf, af, zf, sf, of); globals=layout=kind_0052f44d")
