#include <gp4/gp4.h>

struct UnknownRecord_00432f18 {
    uint8_t Unknown00[0x66];
    int16_t Unknown66;
    uint8_t Unknown68[0x9a - 0x68];
    int16_t Unknown9A;
    uint8_t Unknown9C[0xca - 0x9c];
    uint8_t UnknownCA;
    uint8_t UnknownCB;
    uint8_t UnknownCC[0xe5 - 0xcc];
    uint8_t UnknownE5;
    uint8_t UnknownE6[0x232 - 0xe6];
    uint8_t Unknown232;
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of `cmp left, right` on an operand of the width given by mask / sign bit.
static inline uint32_t compareFlags(uint32_t left, uint32_t right, uint32_t mask, uint32_t sign) {
    left &= mask;
    right &= mask;
    const uint32_t result = (left - right) & mask;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result & sign) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & sign) != 0u ? 0x800u : 0u);
}

// Flags of an `or` / `test` on a byte result: CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// Sets bit 1 in the two status bytes; the flags left behind are those of the last `or`.
static inline uint32_t markBytes(UnknownRecord_00432f18* self, bool useCB) {
    if (useCB) {
        self->UnknownCB |= 2u;
    } else {
        self->UnknownCA |= 2u;
    }
    self->Unknown232 |= 2u;
    return logicFlags8(self->Unknown232);
}

// FUNCTION: GP4 0x00432f18
void __cdecl FUN_00432f18(gp4::Regs* r) {
    UnknownRecord_00432f18* self = reinterpret_cast<UnknownRecord_00432f18*>(static_cast<uintptr_t>(r->esi));

    gp4::call_regs(0x00432c21, *r);
    const int32_t index = static_cast<int32_t>(r->eax);

    uint32_t flags = compareFlags(static_cast<uint32_t>(index), 0u, 0xffffffffu, 0x80000000u);
    bool carry = false;

    if (index < 0) {
        carry = true;
    } else if (index == 0) {
        const int32_t speed = self->Unknown66;
        r->eax = static_cast<uint32_t>(speed);
        const uint32_t selector = self->UnknownE5 & 1u;
        flags = logicFlags8(selector);
        const int32_t limit = (selector == 0u) ? GP4_GLOBAL(int32_t, 0x0062787c) : GP4_GLOBAL(int32_t, 0x00627880);
        flags = compareFlags(static_cast<uint32_t>(speed), static_cast<uint32_t>(limit), 0xffffffffu, 0x80000000u);
        if (speed >= limit) {
            carry = true;
        } else {
            const uint16_t angle = static_cast<uint16_t>(self->Unknown9A);
            r->eax = (r->eax & 0xffff0000u) | angle;
            const int16_t threshold = (selector == 0u) ? 0x200 : 0x100;
            flags = compareFlags(angle, static_cast<uint16_t>(threshold), 0xffffu, 0x8000u);
            if (static_cast<int16_t>(angle) >= threshold) {
                flags = markBytes(self, false);
            }
        }
    } else if (index == 1) {
        flags = compareFlags(static_cast<uint32_t>(index), 1u, 0xffffffffu, 0x80000000u);
        const uint16_t angle = static_cast<uint16_t>(self->Unknown9A);
        r->eax = (r->eax & 0xffff0000u) | angle;
        flags = compareFlags(angle, 0x600u, 0xffffu, 0x8000u);
        if (static_cast<int16_t>(angle) >= 0x600) {
            flags = markBytes(self, false);
        } else {
            flags = compareFlags(angle, 0x300u, 0xffffu, 0x8000u);
            if (static_cast<int16_t>(angle) >= 0x300) {
                flags = markBytes(self, true);
            }
        }
    } else {
        flags = compareFlags(static_cast<uint32_t>(index), 2u, 0xffffffffu, 0x80000000u);
        if (index <= 2) {
            const uint16_t angle = static_cast<uint16_t>(self->Unknown9A);
            r->eax = (r->eax & 0xffff0000u) | angle;
            flags = compareFlags(angle, 0x600u, 0xffffu, 0x8000u);
            if (static_cast<int16_t>(angle) >= 0x600) {
                flags = markBytes(self, true);
            }
        }
    }

    // clc / stc replace CF; the other flags stay from the last flag writer
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d4u) | (carry ? 1u : 0u);
}
GP4_IMPL(0x00432f18, FUN_00432f18, "regs(esi:ptr[0x240]:layout=rec_00432f18) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_00432f18")
