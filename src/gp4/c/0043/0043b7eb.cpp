#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t AddFlags(uint32_t lhs, uint32_t rhs, uint32_t result) {
    return (result < lhs ? 1u : 0u) | parityFlag(result) | ((lhs ^ rhs ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((~(lhs ^ rhs) & (lhs ^ result) & 0x80000000u) >> 20);
}

// Flags of the final `add esi, 0x41c` (esi = record after the step); kept out of line so the end
// address is not folded into an absolute constant.
static __declspec(noinline) uint32_t stepFlags(uint32_t after) {
    return AddFlags(after - 0x41cu, 0x41cu, after);
}

// Scans the 22 car records (stride 0x41c) at 0x0066fef0 and leaves esi at the one with the largest
// lateral offset (signed dx:ax of 0x0043b482, which is skipped and counted 0 for a record whose byte
// +0x8d has bit 1 set); the first record wins ties because the update needs offset >= best, the
// initial best being INT_MIN. Every other register is preserved; the flags are those of the final
// `add esi, 0x41c` of the loop.
// FUNCTION: GP4 0x0043b7eb
void __cdecl FUN_0043b7eb(gp4::Regs* r) {
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t best = 0x80000000u;      // ebp
    uint32_t bestRecord = 0x0066fef0u; // ebx
    uint32_t record = 0x0066fef0u;     // esi

    for (uint32_t remaining = 22u; remaining != 0u; --remaining) {
        eax &= 0xffff0000u;  // xor ax, ax
        edx &= 0xffff0000u;  // xor dx, dx
        if ((GP4_FIELD(uint8_t, record, 0x8d) & 2u) == 0u) {
            gp4::Regs call = *r;
            call.eax = eax;
            call.ebx = bestRecord;
            call.ecx = remaining;
            call.edx = edx;
            call.esi = record;
            call.ebp = best;
            gp4::call_regs(0x0043b482, call);
            eax = call.eax;
            edx = call.edx;
        }
        // shl eax, 16; mov ax, dx; rol eax, 16 -> dx:ax as a 32-bit value
        eax = (edx << 16) | (eax & 0xffffu);
        if (static_cast<int32_t>(eax) >= static_cast<int32_t>(best)) {
            best = eax;
            bestRecord = record;
        }
        record += 0x41cu;
    }

    r->esi = bestRecord;
    r->eflags = (r->eflags & ~0x8d5u) | stepFlags(record);
}
GP4_IMPL(0x0043b7eb, FUN_0043b7eb, "regs() -> (esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+lap_0043b482")
