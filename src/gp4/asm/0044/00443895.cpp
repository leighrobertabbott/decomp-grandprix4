#include <gp4/gp4.h>

// esi: a car record (only the fields read here are named).
struct Unknown_00443895_Car {
    uint8_t Unknown00[0x1c];
    uint32_t Unknown1C;           // current track-segment record
    uint8_t Unknown20[0x7a - 0x20];
    uint8_t Flags7A;
    uint8_t Unknown7B[0xe4 - 0x7b];
    uint8_t UnknownE4;            // state kind, compared signed with 2, 5 and 1
    uint8_t UnknownE5[0x232 - 0xe5];
    uint8_t Flags232;
    uint8_t Unknown233[0x25f - 0x233];
    uint8_t Flags25F;
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit `test x, mask` whose result is `result`: CF = OF = AF = 0.
static inline uint32_t testFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// Flags of the 8-bit `cmp left, right`.
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    left &= 0xffu;
    right &= 0xffu;
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 0x01u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// stc / clc keep the other flags of the last flag writer and replace CF.
static inline uint32_t withCarry(uint32_t writerFlags, uint32_t carry) {
    return (writerFlags & ~1u) | carry;
}

// FUNCTION: GP4 0x00443895
// Decides whether the car in esi is skipped by the car loops of 0x004438f8 / 0x00443955: CF = 1 (stc) when
// bit 7 of +0x25f or bit 4 of +0x7a is set, when the kind byte +0xe4 is 2..5, or (mode byte [0x7c2ea9] not
// 0x80) when bit 0 of +0x25f is set, or (mode 0x80, bit 0 of +0x232 set, kind 1, segment +0x1c at or above the
// pit start [0x891500]) when the slot index of 0x00432c21 is not positive. pushal / popal restore every
// register; only the flags of the last writer, with CF replaced, remain.
void __cdecl FUN_00443895(gp4::Regs* r) {
    const Unknown_00443895_Car* car = reinterpret_cast<const Unknown_00443895_Car*>(static_cast<uintptr_t>(r->esi));
    const uint32_t segment = car->Unknown1C;
    const uint32_t kind = car->UnknownE4;
    uint32_t flags;

    if ((car->Flags25F & 0x80u) != 0u) {
        flags = withCarry(testFlags8(car->Flags25F & 0x80u), 1u);
    } else if ((car->Flags7A & 0x10u) != 0u) {
        flags = withCarry(testFlags8(car->Flags7A & 0x10u), 1u);
    } else if (static_cast<int8_t>(kind) >= 2 && static_cast<int8_t>(kind) <= 5) {
        flags = withCarry(cmpFlags8(kind, 5u), 1u);
    } else if (GP4_GLOBAL(uint8_t, 0x007c2ea9) != 0x80u) {
        const uint32_t bit = car->Flags25F & 1u;
        flags = withCarry(testFlags8(bit), bit != 0u ? 1u : 0u);
    } else if ((car->Flags232 & 1u) == 0u) {
        flags = withCarry(testFlags8(0u), 0u);
    } else if (kind != 1u) {
        flags = withCarry(cmpFlags8(kind, 1u), 0u);
    } else if (segment < GP4_GLOBAL(uint32_t, 0x00891500)) {
        flags = withCarry(cmpFlags32(segment, GP4_GLOBAL(uint32_t, 0x00891500)), 0u);
    } else {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        gp4::call_regs(0x00432c21, c);
        flags = withCarry(cmpFlags32(c.eax, 0u), static_cast<int32_t>(c.eax) > 0 ? 0u : 1u);
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00443895, FUN_00443895, "regs(esi:ptr[0x273]:layout=car_004168fd+rec_00432f18) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00432f18+state_004320f2+ring_008943a0")
