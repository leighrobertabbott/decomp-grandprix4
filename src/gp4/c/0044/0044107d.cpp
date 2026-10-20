#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit ADD left, right.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Runs 0x0040fbf3 for every one of the 22 car records (stride 0x41c) at 0x0066fef0 whose byte +0x7a has bit 7
// clear. Leaves esi one past the last record and ecx = 0 (loop); the flags are those of the last
// "add esi, 0x41c".
// FUNCTION: GP4 0x0044107d
void __cdecl FUN_0044107d(gp4::Regs* r) {
    uint32_t record = 0x0066fef0u;
    uint32_t flags = 0u;
    for (uint32_t remaining = 0x16u; remaining != 0u; --remaining) {
        const uint8_t* const car = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(record));
        if ((car[0x7a] & 0x80u) == 0u) {
            gp4::Regs call = *r;
            call.st_in = 0;
            call.st_out = 0;
            call.esi = record;
            call.ecx = remaining;
            gp4::call_regs(0x0040fbf3, call);
        }
        flags = addFlags32(record, 0x41cu);
        record += 0x41cu;
    }
    r->esi = record;
    r->ecx = 0u;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0044107d, FUN_0044107d, "regs() -> (ecx, esi, cf, pf, af, zf, sf, of); globals=layout=carbg_0066fef0+cars_0066fef0+trk_007ad894+circ_007adb50+codes_0043ba12+curves_0062ecb0+div_0060138c+state_0040fbf3")
