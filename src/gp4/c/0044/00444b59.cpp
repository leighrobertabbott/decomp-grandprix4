#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `add left, right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x01u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Calls a register routine with the given eax / ebx / esi and the rest of the caller's registers.
static inline gp4::Regs callWith(const gp4::Regs* r, uint32_t addr, uint32_t eax, uint32_t ebx, uint32_t esi) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = eax;
    c.ebx = ebx;
    c.esi = esi;
    gp4::call_regs(addr, c);
    return c;
}

// Opcode 0 reader of the opcode stream (the 10-byte record that 0x00444ae9 writes): the read cursor
// [0x0062d1a8] (ebx) points at the record. Without a car [0x0066d828] (esi) it ORs the mode bits (+2 & 0x60)
// into [0x008914e0] and applies the flag byte +8 (0x00445248); with a car it unpacks the control byte +1 into
// the car (0x004451b9), stores +2 & 0x9f at +0x6f, ORs +2 & 0x60 into [0x008914e0], copies word +3 to +0x6a and
// bytes +5/+6/+7 to +0x6e/+0x6c/+0x6d, rewrites +0x78 bit 7 from the flag byte +8 (0x00445236) and, when that
// bit ends up clear after having been set, raises 0x006f3b7c/7d to 0xff and 0x00852d70 to 0x80, otherwise
// clears bit 2 of [0x00852d64] (bit 7 was clear before). Both paths finish by feeding the samples
// al = [0x008914e2], ah = +9 to 0x004454ef and advancing the cursor by 10. ax ends as that pair, ebx = the
// old cursor, esi = the car; the flags are those of the final `add [0x0062d1a8], 10`.
// FUNCTION: GP4 0x00444b59
void __cdecl FUN_00444b59(gp4::Regs* r) {
    const uint32_t cursor = GP4_GLOBAL(uint32_t, 0x0062d1a8);
    const uint32_t carAddress = GP4_GLOBAL(uint32_t, 0x0066d828);
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(cursor));

    if (carAddress == 0u) {
        GP4_GLOBAL(uint8_t, 0x008914e0) = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x008914e0) | (record[2] & 0x60u));
        callWith(r, 0x00445248, (r->eax & 0xffffff00u) | record[8], cursor, carAddress);
    } else {
        uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(carAddress));
        gp4::Regs unpacked = callWith(r, 0x004451b9, (r->eax & 0xffffff00u) | record[1], cursor, carAddress);

        const uint32_t control = record[2];
        car[0x6f] = static_cast<uint8_t>(control & 0x9fu);
        GP4_GLOBAL(uint8_t, 0x008914e0) = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x008914e0) | (control & 0x60u));
        GP4_FIELD(uint16_t, car, 0x6a) = GP4_FIELD(uint16_t, record, 3);
        car[0x6e] = record[5];
        car[0x6c] = record[6];
        car[0x6d] = record[7];

        const uint32_t beforeEax = (unpacked.eax & 0xffff0000u) | ((control & 0x60u) << 8) | record[8];
        if ((car[0x78] & 0x80u) == 0u) {
            callWith(r, 0x00445236, beforeEax, cursor, carAddress);
            GP4_GLOBAL(uint32_t, 0x00852d64) = GP4_GLOBAL(uint32_t, 0x00852d64) & 0xfffffffbu;
        } else {
            callWith(r, 0x00445236, beforeEax, cursor, carAddress);
            if ((car[0x78] & 0x80u) == 0u) {
                GP4_GLOBAL(uint8_t, 0x006f3b7c) = 0xff;
                GP4_GLOBAL(uint8_t, 0x006f3b7d) = 0xff;
                GP4_GLOBAL(uint8_t, 0x00852d70) = 0x80;
            }
        }
    }

    const uint32_t samples = (r->eax & 0xffff0000u) | (static_cast<uint32_t>(record[9]) << 8) | GP4_GLOBAL(uint8_t, 0x008914e2);
    callWith(r, 0x004454ef, samples, cursor, carAddress);

    GP4_GLOBAL(uint32_t, 0x0062d1a8) = cursor + 10u;

    r->eax = samples;
    r->ebx = cursor;
    r->esi = carAddress;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(cursor, 10u);
}
GP4_IMPL(0x00444b59, FUN_00444b59, "regs(eax:u32) -> (eax, ebx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0040fa01+stream_0062d1a8")
