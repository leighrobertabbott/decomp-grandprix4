#include <gp4/gp4.h>
#include <gp4/flags.h>

// Counts the 22 per-driver bytes at 0x007c2a48: eax = those with bit 7 set and bit 6 clear, edx =
// those with both bits set. ecx and esi are saved and restored; the flags are those of the last
// `inc esi` of the scan (CF kept from the `test` before it, which clears it).
// FUNCTION: GP4 0x00522da0
void __cdecl FUN_00522da0(gp4::Regs* r) {
    const uint8_t* const table = GP4_ARRAY(uint8_t, 0x007c2a48);
    const uint32_t base = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(table));
    uint32_t first = 0u;
    uint32_t second = 0u;
    uint32_t arith = 0u;
    for (uint32_t i = 0u; i < 0x16u; ++i) {
        const uint8_t value = table[i];
        arith = gp4::flags::logic8(value & 0x80u);
        if ((value & 0x80u) != 0u) {
            arith = gp4::flags::logic8(value & 0x40u);
            if ((value & 0x40u) == 0u) {
                ++first;
            } else {
                ++second;
            }
        }
        arith = gp4::flags::inc32(base + i, arith);
    }
    r->eax = first;
    r->edx = second;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00522da0, FUN_00522da0, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=neg22_00520023")
