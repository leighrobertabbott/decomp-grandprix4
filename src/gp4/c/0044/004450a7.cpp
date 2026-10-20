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

// Consumes one 10-byte record of the opcode stream whose read cursor is the pointer at 0x0062d1a8:
// copies its two dwords to car [0x0066d828] +0xf0/+0x1d8 and its flag byte to bit 0x20 of +0x8d.
// FUNCTION: GP4 0x004450a7
void __cdecl FUN_004450a7(gp4::Regs* r) {
    const uint32_t cursor = GP4_GLOBAL(uint32_t, 0x0062d1a8);
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(cursor));
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0066d828)));

    r->ebx = cursor;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car));
    if (car != nullptr) {
        *reinterpret_cast<uint32_t*>(car + 0xf0) = *reinterpret_cast<const uint32_t*>(record + 1);
        const uint32_t second = *reinterpret_cast<const uint32_t*>(record + 5);
        r->eax = second;
        *reinterpret_cast<uint32_t*>(car + 0x1d8) = second;
        car[0x8d] = static_cast<uint8_t>(car[0x8d] & 0xdfu);
        if (record[9] != 0u) {
            car[0x8d] = static_cast<uint8_t>(car[0x8d] | 0x20u);
        }
    }

    GP4_GLOBAL(uint32_t, 0x0062d1a8) = cursor + 0x0au;

    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(cursor, 0x0au);
}
GP4_IMPL(0x004450a7, FUN_004450a7, "regs(eax:u32) -> (eax, ebx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0040fa01+stream_0062d1a8")
