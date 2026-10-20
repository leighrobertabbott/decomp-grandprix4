#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of a 32-bit "cmp left, right"
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// flags of an 8-bit logic op (and/or): CF = OF = AF = 0
static inline uint32_t logicFlags8(uint8_t result) {
    return parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

static inline uint16_t abs16(uint16_t v) {
    return (v & 0x8000u) ? (uint16_t)(0u - v) : v;
}

// FUNCTION: GP4 0x00413fb0
void __cdecl FUN_00413fb0(gp4::Regs* r) {
    uint8_t* self = (uint8_t*)r->esi;
    uint8_t* other = (uint8_t*)r->edi;
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t ebp = 6u;
    uint32_t flags;

    gp4::Regs callee = *r;
    callee.ebp = ebp;
    gp4::call_regs(0x00413ea0, callee);

    bool adjust = true;   // continue at 0x414090
    if ((callee.eflags & 1u) == 0u) {
        if ((self[0x203] & 2u) == 0u) {
            self[0x203] |= 2u;
            self[0x1f6] = 5u;
        }
        self[0x6f] |= 2u;
    } else {
        self[0x203] &= 0xfdu;
        const int16_t lateral = GP4_FIELD(int16_t, self, 0x90);
        const uint16_t mag = abs16((uint16_t)lateral);
        edx = (edx & 0xffff0000u) | mag;

        bool check = false;   // continue at 0x41406d
        if ((int16_t)mag < GP4_FIELD(int16_t, self, 0xb0) && GP4_GLOBAL(int16_t, 0x00934df4) < 0x1000) {
            check = true;
        } else {
            gp4::Regs side = *r;
            side.eax = eax;
            side.edx = edx;
            side.ebp = ebp;
            side.eflags = callee.eflags;
            gp4::call_regs(0x00414548, side);
            const bool carry = (side.eflags & 1u) != 0u;
            if ((self[0xe7] & 4u) == 0u) {
                if (carry) {
                    eax = (uint32_t)GP4_FIELD(uint16_t, other, 0x62) + 0x50u - 0x300u;
                    check = (int16_t)(uint16_t)eax >= lateral;
                } else {
                    check = lateral < 0;
                }
            } else {
                if (carry) {
                    eax = 0u - ((uint32_t)GP4_FIELD(uint16_t, other, 0x60) + 0x50u - 0x300u);
                    check = (int16_t)(uint16_t)eax <= lateral;
                } else {
                    check = lateral >= 0;
                }
            }
        }

        if (!check) {
            self[0x6f] |= 1u;
        } else {
            const uint16_t speed = GP4_FIELD(uint16_t, self, 0x9a);
            edx = (edx & 0xffff0000u) | speed;
            if ((int16_t)speed < GP4_GLOBAL(int16_t, 0x006013e0)) {
                self[0x71] &= 0xfeu;
                flags = logicFlags8(self[0x71]) | 1u;   // and byte, then stc
                self[0x72] = 1u;
                adjust = false;
            } else {
                self[0x6f] |= 2u;
            }
        }
    }

    if (adjust) {
        const int16_t lateral = GP4_FIELD(int16_t, self, 0x90);
        int32_t excess = (int16_t)(uint16_t)(abs16((uint16_t)lateral) + 0x100u - GP4_FIELD(uint16_t, self, 0xb0));
        if (excess < 0) excess = 0;
        const int64_t p = (int64_t)excess * (int64_t)GP4_GLOBAL(int32_t, 0x006224f8);
        uint32_t steer = (uint32_t)((uint64_t)p >> 14);
        if (lateral >= 0) steer = 0u - steer;
        if (GP4_GLOBAL(int16_t, 0x00934df8) < 0) steer = 0u - steer;
        ebp = steer;

        uint32_t v = (uint32_t)(int32_t)(int16_t)(uint16_t)(GP4_FIELD(uint16_t, self, 0xe) - GP4_FIELD(uint16_t, other, 0x0)) + steer;
        const uint32_t limit = GP4_GLOBAL(uint32_t, 0x006013ac);
        if (v & 0x80000000u) {
            uint32_t m = 0u - v;
            if (!(m < limit)) m = limit;
            v = 0u - m;
        } else if (!(v < limit)) {
            v = limit;
        }
        GP4_FIELD(uint16_t, self, 0xb4) = (uint16_t)v;
        GP4_GLOBAL(uint8_t, 0x00603c34) = 0xffu;
        eax = v;
        edx = (v & 0x80000000u) ? 0u - v : v;
        if ((int32_t)edx < 0x80) {
            flags = cmpFlags32(edx, 0x80u) & ~1u;   // clc
        } else {
            self[0x6f] |= ((int16_t)(uint16_t)v < 0) ? 4u : 8u;
            flags = logicFlags8(self[0x6f]);
        }
    }

    r->eax = eax;
    r->edx = edx;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00413fb0, FUN_00413fb0, "regs(esi:ptr[0x240]:layout=car_00413fb0, edi:ptr[0x68]:bytes, eax:u32, edx:u32) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_0066fe3c+state_00413f28")
