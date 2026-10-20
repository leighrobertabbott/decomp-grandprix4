#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 32-bit `add left, right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Walks the 22 car records at 0x0066fef0 (stride 0x41c). A record whose byte +0x21c bit 5 and byte +0xfe
// bit 5 are clear and whose byte +0x7b bit 1 is set gets bit 0 of byte +0x162 cleared and bit 5 of byte
// +0xfe set, then 0x0044351d (esi = record) is called. ecx is saved and restored; esi ends one past the
// array; eax / edx hold the results of the last call; the flags are those of the final `add esi, 0x41c`.
// FUNCTION: GP4 0x00443565
void __cdecl FUN_00443565(gp4::Regs* r) {
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t esi = 0x0066fef0u;
    uint32_t before = esi;
    for (int remaining = 0x16; remaining != 0; --remaining) {
        uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(esi));
        if ((record[0x21c] & 0x20u) == 0u && (record[0x7b] & 2u) != 0u && (record[0xfe] & 0x20u) == 0u) {
            record[0x162] = static_cast<uint8_t>(record[0x162] & 0xfeu);
            record[0xfe] = static_cast<uint8_t>(record[0xfe] | 0x20u);
            gp4::Regs call = *r;
            call.eax = eax;
            call.edx = edx;
            call.esi = esi;
            gp4::call_regs(0x0044351d, call);
            eax = call.eax;
            edx = call.edx;
        }
        before = esi;
        esi += 0x41cu;
    }
    r->eax = eax;
    r->edx = edx;
    r->esi = esi;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(before, 0x41cu);
}
GP4_IMPL(0x00443565, FUN_00443565, "regs(eax:u32, edx:u32) -> (eax, edx, esi, cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0+state_0042595c+cars_0066fef0+carst_0040f6d0+flags_00408f7b")
