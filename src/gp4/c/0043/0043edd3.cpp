#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x0043edd3
void __cdecl FUN_0043edd3(gp4::Regs* r) {
    const uint32_t record = r->edi;
    const uint32_t lowerBound = GP4_GLOBAL(uint32_t, 0x00891500);

    gp4::Regs call = *r;
    gp4::call_regs(0x0043fab0, call);
    const uint32_t first = call.eax;

    uint32_t other = record - 0x160u;
    if (record >= lowerBound) {
        if (other < lowerBound) {
            other = lowerBound;
        }
    } else if (other < GP4_GLOBAL(uint32_t, 0x00891504)) {
        other += GP4_GLOBAL(uint32_t, 0x00866ec4);
    }

    call.edi = other;
    gp4::call_regs(0x0043fab0, call);
    const uint32_t sum = call.eax + first;
    const uint32_t result = static_cast<uint32_t>(static_cast<int32_t>(sum) >> 1);

    r->eax = result;
    r->edx = first;
    // sar by 1: CF = bit shifted out, OF = 0, PF/ZF/SF from the result.
    r->eflags = (r->eflags & ~0x8d5u)
        | (sum & 1u)
        | parityFlag8(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) << 7);
}
GP4_IMPL(0x0043edd3, FUN_0043edd3, "regs(edi:rec[0x8943a0,0x160,0..25]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=ring_0043edd3")
