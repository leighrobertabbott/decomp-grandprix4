#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x0040ec79
void __cdecl FUN_0040ec79(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    // pushal ... popal restores every register but not the flags left by the last or
    const uint8_t result = static_cast<uint8_t>(record[0x82] | 0x20u | 0x01u);
    record[0x85] = 2;
    record[0x82] = result;
    // or leaves CF=OF=AF=0, ZF=0 (bit 0 is set); PF/SF come from the stored byte
    const uint32_t flags = parityFlag(result) | ((result & 0x80u) != 0u ? 0x80u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040ec79, FUN_0040ec79, "regs(esi:ptr[0x8d]:bytes) -> (cf, pf, af, zf, sf, of)")
