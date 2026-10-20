#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags left by a byte-wide AND: CF = OF = AF = 0, PF/ZF/SF from the result.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x00414146
// Returns CF = 0 only when bit 7 of byte +0xcc is clear, word 0x934df8 is negative and 0x0041416e
// (steering update of the esi car) returns CF = 0; every other path clears bit 3 of byte +0xff and
// returns CF = 1 (stc). The registers are those 0x0041416e leaves, the flags those of the last writer.
void __cdecl FUN_00414146(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t flags = 0;                                              // set below on every path
    bool carry = true;

    if ((GP4_FIELD(uint8_t, car, 0xcc) & 0x80u) == 0u
        && static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x00934df8)) < 0) {
        gp4::call_regs(0x0041416e, *r);
        if ((r->eflags & 1u) == 0u) {
            carry = false;                                           // jae -> clc
            flags = r->eflags & 0x8d4u;
        }
    }

    if (carry) {
        GP4_FIELD(uint8_t, car, 0xff) &= 0xf7;
        flags = logicFlags8(GP4_FIELD(uint8_t, car, 0xff)) | 1u;     // and, then stc
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00414146, FUN_00414146, "regs(eax:u32, edx:u32, ebp:u32, edi:rec[0x8943a0,0x160,0..29], esi:ptr[0x23d]:layout=car_0041416e+ptr_00414439) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=ring_008943a0+state_0041416e")
