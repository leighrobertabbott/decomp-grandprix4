#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of cmp left, right
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// Largest distance (0x004224be: integer square root of the squared distance to record i)
// over the entries i of the dword table behind DAT_0062a048 that match the mask DAT_0062a054;
// entries with any of the 0x14280 bits get +0x300. Starts from 0 and returns it in eax;
// the final flags are those of "cmp result, 0x80". All other registers are preserved.
// FUNCTION: GP4 0x00422501
void __cdecl FUN_00422501(gp4::Regs* r) {
    const uint32_t* entries = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0062a048)));
    const uint32_t mask = GP4_GLOBAL(uint32_t, 0x0062a054);

    int32_t best = 0;
    uint32_t i = 0;
    do {
        if ((entries[i] & mask) != 0u) {
            gp4::Regs call = *r;
            call.eax = i;
            gp4::call_regs(0x004224be, call);
            uint32_t distance = call.eax;
            if ((entries[i] & 0x14280u) != 0u) {
                distance += 0x300u;
            }
            if (best < (int32_t)distance) {
                best = (int32_t)distance;
            }
        }
        i = i + 1u;
    } while (i != GP4_GLOBAL(uint32_t, 0x0062a05c));

    r->eax = (uint32_t)best;
    r->eflags = (r->eflags & ~0x8d5u) | subFlags((uint32_t)best, 0x80u);
}
GP4_IMPL(0x00422501, FUN_00422501, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=list_0062a048")
