#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of a logic result (TEST/OR/AND): CF = OF = AF = 0.
static inline uint32_t logicFlags(uint32_t result, uint32_t signBit) {
    return byteParity(result)
        | (result == 0u ? 0x040u : 0u)
        | ((result & signBit) != 0u ? 0x080u : 0u);
}

// Flags of the 32-bit CMP left, right.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_Car {
    uint8_t Unknown00[0x76];
    uint8_t Flags76;
    uint8_t Unknown77[0x0f];
    uint8_t Flags86;
    uint8_t Unknown87[0x17c];
    uint8_t Flags203;
    uint8_t Unknown204[0x34];
    uint16_t Threshold238;
    uint8_t Unknown23A[6];
    uint32_t Value240;
};

// When the dword at +0x240 is zero and bit 7 of +0x203 is set, rolls the random byte from 0x00401462
// and, if it (times 32, plus 0x2000) reaches the word at +0x238, sets bits in +0x86 and +0x76.
// FUNCTION: GP4 0x00444284
void __cdecl FUN_00444284(gp4::Regs* r) {
    Unknown_Car* car = reinterpret_cast<Unknown_Car*>(static_cast<uintptr_t>(r->esi));

    const uint32_t value = car->Value240;
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags(value, 0x80000000u);
    if (value != 0u) {
        return;
    }

    const uint8_t gate = static_cast<uint8_t>(car->Flags203 & 0x80u);
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags(gate, 0x80u);
    if (gate == 0u) {
        return;
    }

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x00401462, call);

    const uint32_t limit = ((call.eax & 0xffu) << 5) + 0x2000u;
    const uint32_t threshold = car->Threshold238;
    r->eax = limit;
    r->edx = threshold;
    r->eflags = (r->eflags & ~0x8d5u) | cmpFlags32(limit, threshold);
    if (static_cast<int32_t>(limit) < static_cast<int32_t>(threshold)) {
        return;
    }

    car->Flags86 = static_cast<uint8_t>(car->Flags86 | 0x08u);
    car->Flags76 = static_cast<uint8_t>(car->Flags76 | 0x08u);
    car->Flags76 = static_cast<uint8_t>(car->Flags76 | 0x04u);
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags(car->Flags76, 0x80u);
}
GP4_IMPL(0x00444284, FUN_00444284, "regs(eax:u32, esi:ptr[0x35c]:layout=rec_004053ff+rec_00413d01+car_00444284) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0")
