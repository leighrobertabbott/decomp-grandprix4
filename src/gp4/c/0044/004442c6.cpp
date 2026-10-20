#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit logic result (TEST/OR/AND): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return byteParity(result) | (result == 0u ? 0x040u : 0u) | (result & 0x80u);
}

// Flags of the 8-bit CMP left, right.
static inline uint32_t cmpFlags8(uint8_t left, uint8_t right) {
    const uint8_t result = static_cast<uint8_t>(left - right);
    return (left < right ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// Flags of the 16-bit CMP left, right.
static inline uint32_t cmpFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    return (left < right ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x8000u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

struct Unknown_Car {
    uint8_t Unknown00[0x7a];
    uint8_t Flags7a;
    uint8_t Unknown7b[0x0b];
    uint8_t Flags86;
    uint8_t Unknown87[0x13];
    int16_t Value9a;
};

// When the option byte 0x00625ab8 is 0xff, the car's word at +0x9a is at least 0x1d5 and bit 1 of its byte +0x7a is
// clear, sets bits 0x40 and 0x20 in its byte +0x86. Flags are those of the last flag-setting instruction executed.
// FUNCTION: GP4 0x004442c6
void __cdecl FUN_004442c6(gp4::Regs* r) {
    Unknown_Car* car = reinterpret_cast<Unknown_Car*>(static_cast<uintptr_t>(r->esi));
    uint32_t flags;

    const uint8_t option = GP4_GLOBAL(uint8_t, 0x00625ab8);
    flags = cmpFlags8(option, 0xffu);
    if (option == 0xffu) {
        const int16_t value = car->Value9a;
        flags = cmpFlags16(static_cast<uint16_t>(value), 0x1d5u);
        if (value >= 0x1d5) {
            const uint8_t bits = car->Flags7a;
            flags = logicFlags8(bits & 2u);
            if ((bits & 2u) == 0u) {
                car->Flags86 = static_cast<uint8_t>(car->Flags86 | 0x40u);
                car->Flags86 = static_cast<uint8_t>(car->Flags86 | 0x20u);
                flags = logicFlags8(car->Flags86);
            }
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004442c6, FUN_004442c6, "regs(esi:ptr[0xa8]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
