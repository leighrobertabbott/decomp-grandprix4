#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags left by a byte-wide OR: CF = OF = AF = 0, PF/ZF/SF from the result.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x0042784b
// `pushal ; call 0x004036ca ; or byte [esi+0xfe], 8 ; popal ; ret`: runs 0x004036ca on the esi
// record, then sets bit 3 of the byte at +0xfe. Every register is restored; the flags are those
// of the byte OR.
void __cdecl FUN_0042784b(gp4::Regs* r) {
    gp4::Regs saved = *r;
    gp4::call_regs(0x004036ca, *r);
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(saved.esi));
    record[0xfe] = static_cast<uint8_t>(record[0xfe] | 8u);
    saved.eflags = (saved.eflags & ~0x8d5u) | logicFlags8(record[0xfe]);
    *r = saved;
}
GP4_IMPL(0x0042784b, FUN_0042784b, "regs(esi:ptr[0x420]:layout=rec_00409416) -> (cf, pf, af, zf, sf, of); globals=layout=state_00403800")
