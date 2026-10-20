#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit `test` (CF = OF = AF = 0).
static inline uint32_t testFlags8(uint32_t value) {
    value &= 0xffu;
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | (value & 0x80u);
}

// Flags of a 16-bit `test` (CF = OF = AF = 0).
static inline uint32_t testFlags16(uint32_t value) {
    value &= 0xffffu;
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x8000u) != 0u ? 0x80u : 0u);
}

// Flags of the 16-bit `cmp left, right`.
static inline uint32_t cmpFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 0x01u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit `sub left, right`.
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Car record (stride 0x41c at 0x66fef0), only the fields read here.
struct Unknown_00441eb8_Car {
    uint8_t Unknown00[0x1c];
    const uint8_t* Segment1C;       // current track-segment record (word +0x1a: group in bits 5..10)
    uint8_t Unknown20[0x7a - 0x20];
    uint8_t Flags7A;
    uint8_t Unknown7B[0x8d - 0x7b];
    uint8_t Flags8D;
    uint8_t Unknown8E[0xcc - 0x8e];
    uint8_t FlagsCC;
};

// Round-robin over the car records and the segment groups. [0x66d890] is the current group (4-byte entries at
// 0x66fd70: byte 0 bit 6, word +2 in use), [0x66d892] the byte offset of the car to examine next. While the
// group is in use and not flagged, ONE car per call is examined: if it is eligible (byte +0xcc bit 6, byte +0x7a
// bits 7 and 6 clear, +0x8d bit 1 clear) and its segment's group (word +0x1a bits 5..10, plus 1) or one of the three
// groups before it (wrapping at the group count [0x868556]) equals the current group, the selection moves on;
// otherwise the offset advances by one car (after the last car the group is switched off through 0x441e18 and its
// bit marked through 0x441c83, and the selection moves on). Moving on resets the car offset and advances the group
// to the next one in use (at most three tries). pushal / popal restore every register; the flags are those of the
// last flag writer.
// FUNCTION: GP4 0x00441eb8
void __cdecl FUN_00441eb8(gp4::Regs* r) {
    const uint32_t groupCount = GP4_GLOBAL(uint16_t, 0x00868556);
    const uint32_t group = GP4_GLOBAL(uint16_t, 0x0066d890);
    const uint32_t tableOffset = (group << 2) & 0xffffu;
    const uint8_t* entry = GP4_ARRAY(const uint8_t, 0x0066fd70) + tableOffset;

    uint32_t flags = 0;
    bool moveOn = false;
    uint32_t esi = 0;
    uint32_t edi = r->edi;

    const uint32_t active = *reinterpret_cast<const uint16_t*>(entry + 2);
    if (active == 0u) {
        moveOn = true;
    } else if ((entry[0] & 0x40u) != 0u) {
        moveOn = true;
    } else {
        esi = 0x0066fef0u + static_cast<uint32_t>(static_cast<int32_t>(GP4_GLOBAL(int16_t, 0x0066d892)));
        const Unknown_00441eb8_Car* car = reinterpret_cast<const Unknown_00441eb8_Car*>(static_cast<uintptr_t>(esi));
        if ((car->FlagsCC & 0x40u) != 0u && (car->Flags7A & 0x80u) == 0u && (car->Flags7A & 0x40u) == 0u
            && (car->Flags8D & 2u) == 0u) {
            edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car->Segment1C));
            uint32_t ax = (((static_cast<uint32_t>(*reinterpret_cast<const uint16_t*>(car->Segment1C + 0x1a)) & 0x7ffu) >> 5) + 1u) & 0xffffu;
            if (!(static_cast<int16_t>(ax) < static_cast<int16_t>(groupCount))) {
                ax = (ax - groupCount) & 0xffffu;
            }
            if (ax == group) {
                moveOn = true;
            }
            for (int k = 0; k < 3 && !moveOn; ++k) {
                ax = (ax - 1u) & 0xffffu;
                if ((ax & 0x8000u) != 0u) {
                    ax = (ax + groupCount) & 0xffffu;
                }
                if (ax == group) {
                    moveOn = true;
                }
            }
        }
        if (!moveOn) {
            esi += 0x41cu;
            if (esi != 0x00675958u) {
                GP4_GLOBAL(uint16_t, 0x0066d892) = static_cast<uint16_t>(esi - 0x0066fef0u);
                flags = subFlags32(esi, 0x0066fef0u);
                r->eflags = (r->eflags & ~0x8d5u) | flags;
                return;
            }
            // The last car was examined without a match: switch the group off and mark its bit.
            gp4::Regs c = *r;
            c.st_in = 0;
            c.st_out = 0;
            c.eax = (r->eax & 0xffff0000u) | group;
            c.ebx = 0x0066fd70u + tableOffset;
            c.ecx = tableOffset;
            c.edx = (r->edx & 0xffff0000u) | 0xffffu;
            c.esi = esi;
            c.edi = edi;
            gp4::call_regs(0x00441e18, c);
            c.eax = group;
            gp4::call_regs(0x00441c83, c);
            moveOn = true;
        }
    }

    // Move on: restart at car 0 and take the next group in use (three tries).
    GP4_GLOBAL(uint16_t, 0x0066d892) = 0;
    uint32_t ax = GP4_GLOBAL(uint16_t, 0x0066d890);
    for (int tries = 0; tries < 3; ++tries) {
        ax = (ax + 1u) & 0xffffu;
        if (!(static_cast<int16_t>(ax) < static_cast<int16_t>(groupCount))) {
            ax = (ax - groupCount) & 0xffffu;
        }
        const uint32_t offset = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>((ax << 2) & 0xffffu)));
        const uint32_t word = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(0x0066fd70u + offset + 2u));
        flags = testFlags16(word);
        if (word != 0u) {
            break;
        }
        if (tries == 2) {
            flags = cmpFlags16(0u, 1u);  // sub dx,1 from 0
        }
    }
    GP4_GLOBAL(uint16_t, 0x0066d890) = static_cast<uint16_t>(ax);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00441eb8, FUN_00441eb8, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+carcc_0043a785+ring_008943a0+records_008943a0+segw_00441e25+state_00441eb8")
