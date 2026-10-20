#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit logic result (test / or): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Flags of the 8-bit CMP left, right.
static inline uint32_t cmpFlags8(uint8_t left, uint8_t right) {
    const uint8_t result = static_cast<uint8_t>(left - right);
    return (left < right ? 0x001u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// Flags of the 16-bit CMP left, right.
static inline uint32_t cmpFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    return (left < right ? 0x001u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x8000u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

static inline uint8_t* recordAt(uint32_t address) {
    return reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(address));
}

// Moves the segment record edi by `cx` records (0x406045 forward for cx >= 0, 0x406077 backward for cx < 0; the
// callees also leave cx in c.ecx), then keeps stepping while the record is marked busy (bit 7 of +0x48) and its
// state word +0x1e is not 2.
static inline void setCx(gp4::Regs& c, int16_t value) {
    c.ecx = (c.ecx & 0xffff0000u) | static_cast<uint16_t>(value);
}

template <uint32_t kRoutine>
static void walkRecords(gp4::Regs& c, bool forward) {
    int16_t steps = static_cast<int16_t>(c.ecx);
    if (forward) {
        steps = static_cast<int16_t>(steps - 1);
        setCx(c, steps);
        if (steps < 0) {
            steps = -1;
        } else {
            do {
                gp4::call_regs(kRoutine, c);
                steps = static_cast<int16_t>(static_cast<int16_t>(c.ecx) - 1);
                setCx(c, steps);
            } while (steps >= 0);
        }
    } else {
        steps = static_cast<int16_t>(static_cast<int16_t>(0 - steps) - 1);
        setCx(c, steps);
        do {
            gp4::call_regs(kRoutine, c);
            steps = static_cast<int16_t>(static_cast<int16_t>(c.ecx) - 1);
            setCx(c, steps);
        } while (steps >= 0);
    }
    for (;;) {
        uint8_t* record = recordAt(c.edi);
        if ((GP4_FIELD(uint8_t, record, 0x48) & 0x80u) == 0u) {
            break;
        }
        if (GP4_FIELD(uint16_t, record, 0x1e) == 2u) {
            break;
        }
        gp4::call_regs(kRoutine, c);
    }
}

// Starts a one-shot effect on the segment the car (esi) stands on, `cx` records ahead (cx < 0: behind) with the
// kind byte al stored at [0x622446]: skipped when bit 1 of car byte +0x8d is set, when the segment (dword +0x1c)
// has bit 5 of +0x1b, or when the stepped-to record's slot (word +0x1a: bits 5..10) is beyond the slot count
// [0x868556] or already used by another kind (table 0x66fd70, 4 bytes per slot; 0x441e18 retires it first).
// Otherwise it claims the slot, marks the record busy (+0x48 bit 7, +0x34 and +0x38 bit 7, state word +0x1e = 2)
// and runs 0x441d18 / 0x441c0c, then 0x441d7a unless the debug flag [0x66d872] is set. pushal/popal keep every
// register; the flags are those of the last flag writer on the path taken.
// FUNCTION: GP4 0x00441af6
void __cdecl FUN_00441af6(gp4::Regs* r) {
    uint8_t* car = recordAt(r->esi);
    uint32_t flags;

    flags = logicFlags8(GP4_FIELD(uint8_t, car, 0x8d) & 2u);
    if ((GP4_FIELD(uint8_t, car, 0x8d) & 2u) != 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | flags;
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    GP4_GLOBAL(uint8_t, 0x00622446) = static_cast<uint8_t>(c.eax);
    c.edi = GP4_FIELD(uint32_t, car, 0x1c);

    flags = logicFlags8(GP4_FIELD(uint8_t, recordAt(c.edi), 0x1b) & 0x20u);
    if ((GP4_FIELD(uint8_t, recordAt(c.edi), 0x1b) & 0x20u) != 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | flags;
        return;
    }

    if (static_cast<int16_t>(c.ecx) >= 0) {
        walkRecords<0x00406045u>(c, true);
    } else {
        walkRecords<0x00406077u>(c, false);
    }

    uint8_t* record = recordAt(c.edi);
    flags = logicFlags8(GP4_FIELD(uint8_t, record, 0x1b) & 0x20u);
    if ((GP4_FIELD(uint8_t, record, 0x1b) & 0x20u) != 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | flags;
        return;
    }

    const uint32_t slot = (GP4_FIELD(uint32_t, record, 0x1a) & 0x7ffu) >> 5;
    c.ecx = slot;
    flags = cmpFlags16(static_cast<uint16_t>(slot), GP4_GLOBAL(uint16_t, 0x00868556));
    if (static_cast<int16_t>(slot) >= static_cast<int16_t>(GP4_GLOBAL(uint16_t, 0x00868556))) {
        r->eflags = (r->eflags & ~0x8d5u) | flags;
        return;
    }
    GP4_GLOBAL(uint32_t, 0x0062fc0c) = slot;
    c.ecx = slot << 2;
    c.ebx = 0x0066fd70u + c.ecx;
    uint8_t* entry = recordAt(c.ebx);

    if (GP4_FIELD(uint16_t, entry, 2) != 0u) {
        flags = cmpFlags8(GP4_FIELD(uint8_t, entry, 1), 0u);
        if (GP4_FIELD(uint8_t, entry, 1) == 0u) {
            r->eflags = (r->eflags & ~0x8d5u) | flags;
            return;
        }
        flags = cmpFlags8(GP4_GLOBAL(uint8_t, 0x00622446), 0u);
        if (GP4_GLOBAL(uint8_t, 0x00622446) != 0u) {
            r->eflags = (r->eflags & ~0x8d5u) | flags;
            return;
        }
        c.eax = (c.eax & 0xffff0000u) | ((c.ecx & 0xffffu) >> 2);
        gp4::call_regs(0x00441e18, c);
        record = recordAt(c.edi);
        entry = recordAt(c.ebx);
    }

    const uint32_t word = GP4_FIELD(uint16_t, record, 0x1a);
    GP4_FIELD(uint8_t, entry, 0) = static_cast<uint8_t>(word & 0x1fu);
    GP4_FIELD(uint8_t, entry, 1) = GP4_GLOBAL(uint8_t, 0x00622446);
    c.eax = (c.eax & 0xffff0000u) | (word & 0xff00u) | GP4_GLOBAL(uint8_t, 0x00622446);

    GP4_FIELD(uint8_t, record, 0x48) = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x48) & 0xbfu);
    GP4_FIELD(uint16_t, record, 0x1e) = 2u;
    GP4_FIELD(uint8_t, record, 0x48) = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x48) | 0x80u);
    GP4_FIELD(uint8_t, record, 0x34) = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x34) | 0x80u);
    GP4_FIELD(uint8_t, record, 0x38) = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x38) | 0x80u);

    gp4::call_regs(0x00441d18, c);
    gp4::call_regs(0x00441c0c, c);
    record = recordAt(c.edi);
    entry = recordAt(c.ebx);

    if ((GP4_FIELD(uint8_t, entry, 0) & 0x80u) != 0u) {
        GP4_FIELD(uint8_t, record, 0x48) = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x48) | 0x40u);
    }
    if (GP4_GLOBAL(uint8_t, 0x0066d872) != 0u) {
        GP4_FIELD(uint8_t, entry, 0) = static_cast<uint8_t>(GP4_FIELD(uint8_t, entry, 0) | 0x40u);
        flags = logicFlags8(GP4_FIELD(uint8_t, entry, 0));
    } else {
        gp4::call_regs(0x00441d7a, c);
        flags = c.eflags & 0x8d5u;
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00441af6, FUN_00441af6, "regs(eax:u32, ecx:i32[-8..8], edx:u32, esi:ptr[568]:layout=car_0043c677) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+records_008943a0+segw_00441e25+state_00441c0c+seg_00441af6")
