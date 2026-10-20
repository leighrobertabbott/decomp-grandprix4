#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit SUB / CMP left, right.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Per-car step of a round-robin walk over the 22 car records at 0x0066fef0: the cursor [0x0066d88e] is the byte
// offset of the current car; 0x0040b5f1, 0x004087aa and 0x0044182d run on it, its word +0xa2 receives the larger
// of ecx (from 0x0044182d) and [0x00602fb0], 0x004414a8 runs unless bit 2 of byte +0xe5 is set, and the cursor
// advances one record (wrapping to the first at the end 0x00675958). esi leaves as the new cursor, the flags are
// those of the final `sub esi, 0x66fef0`.
// FUNCTION: GP4 0x00435280
void __cdecl FUN_00435280(gp4::Regs* r) {
    uint32_t car = GP4_GLOBAL(uint16_t, 0x0066d88e) + 0x0066fef0u;
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(car));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.esi = car;
    gp4::call_regs(0x0040b5f1, c);
    gp4::call_regs(0x004087aa, c);
    gp4::call_regs(0x0044182d, c);

    const int32_t floorValue = GP4_GLOBAL(int32_t, 0x00602fb0);
    if (static_cast<int32_t>(c.ecx) < floorValue) {
        c.ecx = static_cast<uint32_t>(floorValue);
    }
    GP4_FIELD(uint16_t, record, 0xa2) = static_cast<uint16_t>(c.ecx);

    if ((GP4_FIELD(uint8_t, record, 0xe5) & 4u) == 0u) {
        gp4::call_regs(0x004414a8, c);
    }

    car += 0x41cu;
    if (!(car < 0x00675958u)) {
        car = 0x0066fef0u;
    }
    const uint32_t flags = cmpFlags32(car, 0x0066fef0u);
    car -= 0x0066fef0u;
    GP4_GLOBAL(uint16_t, 0x0066d88e) = static_cast<uint16_t>(car);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->ebp = c.ebp;
    r->esi = car;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00435280, FUN_00435280, "regs() -> (eax, ebx, ecx, edx, ebp, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=search_0066d82c+state_004087aa+state_0043c1ba+state_004320f2+counter_0040f0dd+scale_0066ec60")
