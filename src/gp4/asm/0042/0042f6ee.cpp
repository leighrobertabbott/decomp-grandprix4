#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 16-bit `cmp left, right`.
static inline uint32_t cmpFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Flags of an 8-bit logic result (CF = OF = AF = 0).
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | (result & 0x80u);
}

struct Unknown_0042f6ee_Car {
    uint8_t Unknown00[0x8d];
    uint8_t Flags8D;
    uint8_t Unknown8E[0x90 - 0x8e];
    int16_t Unknown90;
    uint8_t Unknown92[0x9c - 0x92];
    uint8_t Flags9C;
    uint8_t Unknown9D[0xea - 0x9d];
    uint16_t StateEA;
    uint8_t UnknownEC[0xfe - 0xec];
    uint8_t FlagsFE;
    uint8_t UnknownFF[0x135 - 0xff];
    uint8_t Flags135;
    uint8_t Unknown136[0x1bc - 0x136];
    uint16_t Unknown1BC;
    uint8_t Unknown1BE[0x1d4 - 0x1be];
    uint32_t Unknown1D4;
    uint8_t Unknown1D8[0x203 - 0x1d8];
    uint8_t Flags203;
};

// Drops the record's tracking state: clears bit 6 of +0x135 and resets +0x1bc / +0x1d4.
static inline uint32_t dropTracking(Unknown_0042f6ee_Car* car) {
    car->Flags135 = static_cast<uint8_t>(car->Flags135 & 0xbfu);
    car->Unknown1BC = 0xffffu;
    car->Unknown1D4 = 0u;
    return logicFlags8(car->Flags135);
}

// FUNCTION: GP4 0x0042f6ee
// Updates the state word +0xea of record esi (0x18 / 0x20 / 0). With bit 1 of +0x9c set (and
// no bit 6 in +0xfe) it becomes 0x18 unless it already is (dropping the tracking state when it
// was neither 0x18 nor 0x20). With bit 1 of +0x8d set instead it becomes 0x20 when the word +0x90
// (negated when [0x868582] is negative) reaches [0x603b1a], else 0x18; with neither bit it is
// cleared to 0 (when it was 0x18 or 0x20). Bit 6 of +0xfe calls the reset 0x0042f9f8 instead.
// Only ax (and dx in the +0x8d case) change; the flags are those of the last flag writer.
void __cdecl FUN_0042f6ee(gp4::Regs* r) {
    Unknown_0042f6ee_Car* car = reinterpret_cast<Unknown_0042f6ee_Car*>(static_cast<uintptr_t>(r->esi));
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t flags;
    uint32_t state;

    if ((car->Flags9C & 2u) != 0u || (car->Flags8D & 2u) != 0u) {
        if ((car->FlagsFE & 0x40u) != 0u) {
            gp4::Regs call = *r;
            call.st_in = 0;
            call.st_out = 0;
            gp4::call_regs(0x0042f9f8, call);
            r->eax = call.eax;
            r->ebx = call.ebx;
            r->ecx = call.ecx;
            r->edx = call.edx;
            r->edi = call.edi;
            r->ebp = call.ebp;
            r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
            return;
        }

        if ((car->Flags9C & 2u) != 0u) {
            if (car->StateEA == 0x18u) {
                r->eflags = (r->eflags & ~0x8d5u) | cmpFlags16(car->StateEA, 0x18u);
                return;
            }
            flags = cmpFlags16(car->StateEA, 0x20u);
            if (car->StateEA != 0x20u) {
                flags = dropTracking(car);
            }
            state = 0x18u;
        } else {
            flags = cmpFlags16(car->StateEA, 0x20u);
            if (car->StateEA != 0x18u && car->StateEA != 0x20u) {
                flags = dropTracking(car);
            }
            state = 0x18u;
            uint32_t dx = static_cast<uint16_t>(car->Unknown90);
            if ((GP4_GLOBAL(uint8_t, 0x00868582) & 0x80u) != 0u) {
                dx = static_cast<uint16_t>(0u - dx);
            }
            edx = (edx & 0xffff0000u) | dx;
            flags = cmpFlags16(dx, static_cast<uint16_t>(GP4_GLOBAL(int16_t, 0x00603b1a)));
            if (static_cast<int16_t>(dx) >= GP4_GLOBAL(int16_t, 0x00603b1a)) {
                state = 0x20u;
            }
        }
    } else {
        flags = cmpFlags16(car->StateEA, 0x20u);
        if (car->StateEA != 0x18u && car->StateEA != 0x20u) {
            r->eflags = (r->eflags & ~0x8d5u) | flags;
            return;
        }
        car->Flags203 = static_cast<uint8_t>(car->Flags203 & 0xf7u);
        car->Unknown1BC = 0xffffu;
        car->Unknown1D4 = 0u;
        flags = logicFlags8(car->Flags203);
        state = 0u;
    }

    car->StateEA = static_cast<uint16_t>(state);
    r->eax = (eax & 0xffff0000u) | state;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042f6ee, FUN_0042f6ee, "regs(eax:u32, edx:u32, esi:rec[0x66fef0,0x41c,0..21]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800+pair_0042e8e6+cars_0066fef0+links_00405a12+div_00403bb6+ringw_004291f4+carf_004036ca+carea_0042f6ee")
