#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit ADD left, right.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Appends one 10-byte record for the car [0x0066d820] to the opcode stream whose write cursor is
// the pointer at 0x0062d1a0: opcode (al), car byte +0x7c | 0x40, state bits (0x445185), words
// +0xf6/+0x10e and byte [0x0066d8b4]. CF is set (nothing written) when there is no car.
// FUNCTION: GP4 0x004450e5
void __cdecl FUN_004450e5(gp4::Regs* r) {
    const uint8_t* car = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0066d820)));
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car));
    if (car == nullptr) {
        r->eflags = (r->eflags & ~0x8d5u) | 0x045u;
        return;
    }

    const uint32_t cursor = GP4_GLOBAL(uint32_t, 0x0062d1a0);
    uint8_t* stream = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(cursor));
    r->ebx = cursor;
    stream[0] = static_cast<uint8_t>(r->eax);
    const uint8_t kind = static_cast<uint8_t>(car[0x7c] | 0x40u);
    stream[1] = kind;

    gp4::Regs c = *r;
    c.eax = (r->eax & 0xffffff00u) | kind;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00445185, c);
    stream[4] = static_cast<uint8_t>(c.eax);

    *reinterpret_cast<uint16_t*>(stream + 5) = *reinterpret_cast<const uint16_t*>(car + 0xf6);
    *reinterpret_cast<uint16_t*>(stream + 7) = *reinterpret_cast<const uint16_t*>(car + 0x10e);
    const uint8_t last = GP4_GLOBAL(uint8_t, 0x0066d8b4);
    stream[9] = last;
    r->eax = (c.eax & 0xffffff00u) | last;

    GP4_GLOBAL(uint32_t, 0x0062d1a0) = cursor + 0x0au;
    r->eflags = (r->eflags & ~0x8d5u) | (addFlags32(cursor, 0x0au) & ~1u);
}
GP4_IMPL(0x004450e5, FUN_004450e5, "regs(eax:u32) -> (eax, ebx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0041104f+flags_00408f7b+carx_0042fe2f+stream_0062d1a0+rec_004450e5")
