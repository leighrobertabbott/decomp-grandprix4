#include <gp4/gp4.h>

static inline uint32_t parityEvenFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// SF/ZF/PF of a byte logic operation (and/or); CF = OF = AF = 0
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityEvenFlag(result)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80u) != 0u ? 0x080u : 0u);
}

// FUNCTION: GP4 0x00413f28
// CF set = the car is in the overtaking-band checks and FUN_00413ea0 found a
// qualifying record (bit 1 of +0x203 / +0x6f raised); CF clear = bit 1 of +0x203
// cleared.
void __cdecl FUN_00413f28(gp4::Regs* r) {
    uint8_t* const self = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t edxHigh = r->edx & 0xffff0000u;

    const int16_t speedLike = GP4_FIELD(int16_t, self, 0x90);
    const int16_t limitB0 = GP4_FIELD(int16_t, self, 0xb0);

    int16_t dx = speedLike;
    if (dx < 0) dx = static_cast<int16_t>(-dx);

    bool pass = false;
    uint32_t ebp = r->ebp;
    uint32_t callFlags = 0;

    dx = static_cast<int16_t>(dx - 0x100);
    if (dx >= limitB0) {
        dx = static_cast<int16_t>(dx - 0x500);
        if (dx < limitB0) {
            dx = GP4_FIELD(int16_t, self, 0x9a);
            if (dx < 0x755) {
                dx = static_cast<int16_t>(speedLike ^ GP4_GLOBAL(int16_t, 0x00934df0));
                if (dx < 0) {
                    if (GP4_GLOBAL(int16_t, 0x00934df4) >= 0xc00) {
                        ebp = 4;
                        gp4::Regs callee = *r;
                        callee.edx = edxHigh | static_cast<uint16_t>(dx);
                        callee.ebp = ebp;
                        gp4::call_regs(0x00413ea0, callee);
                        callFlags = callee.eflags;
                        pass = (callFlags & 1u) == 0u;
                    }
                }
            }
        }
    }

    uint32_t flags;
    if (pass) {
        if ((self[0x203] & 2u) == 0u) {
            self[0x203] = static_cast<uint8_t>(self[0x203] | 2u);
            self[0x1f6] = 5;
        }
        self[0x6f] = static_cast<uint8_t>(self[0x6f] | 2u);
        flags = logicFlags8(self[0x6f]) | 1u;   // or ...; stc
    } else {
        self[0x203] = static_cast<uint8_t>(self[0x203] & 0xfdu);
        flags = logicFlags8(self[0x203]);       // and ...; clc
    }

    r->edx = edxHigh | static_cast<uint16_t>(dx);
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00413f28, FUN_00413f28, "regs(edx:u32, esi:rec[0x66fef0,0x41c,0..21]) -> (edx, ebp, cf, pf, af, zf, sf, of); globals=layout=list_0066fe3c+state_00413f28")
