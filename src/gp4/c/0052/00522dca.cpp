#include <gp4/gp4.h>
#include <gp4/flags.h>

// Finds the first of the 22 per-driver bytes at 0x007c2a48 that has bit 7 set and bit 6 clear and
// returns it in al; al = 1 when there is none. esi and ecx are saved and restored. The flags are
// those of the `test byte, 0x40` that accepted the entry, or of the last `inc esi` of the scan.
// FUNCTION: GP4 0x00522dca
void __cdecl FUN_00522dca(gp4::Regs* r) {
    const uint8_t* table = GP4_ARRAY(uint8_t, 0x007c2a48);
    const uint32_t base = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(table));
    uint8_t found = 1u;
    uint32_t arith = 0u;
    for (uint32_t i = 0; i != 0x16u; ++i) {
        const uint8_t value = table[i];
        arith = gp4::flags::logic8(value & 0x80u);
        if ((value & 0x80u) != 0u) {
            arith = gp4::flags::logic8(value & 0x40u);
            if ((value & 0x40u) == 0u) {
                found = value;
                break;
            }
        }
        arith = gp4::flags::inc32(base + i, arith);
    }
    r->eax = (r->eax & 0xffffff00u) | found;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00522dca, FUN_00522dca, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=neg22_00520023")
