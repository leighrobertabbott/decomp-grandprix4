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

// When the high word of the dword at car +0x244 is at least 0x7d and bit 3 of the byte 0x007ab540 is set, sets bit 2
// of the car bytes +0x161 and +0xcd and the mode byte +0x84 to 3. eax is the high word (its low byte becomes 3 in
// the full path); the flags are those of the last flag-setting instruction executed.
// FUNCTION: GP4 0x00440df7
void __cdecl FUN_00440df7(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    uint32_t value = *reinterpret_cast<const uint32_t*>(car + 0x244) >> 16;
    uint32_t flags = cmpFlags32(value, 0x7du);
    if (static_cast<int32_t>(value) >= 0x7d) {
        const uint8_t gate = GP4_GLOBAL(uint8_t, 0x007ab540);
        flags = logicFlags8(gate & 8u);
        if ((gate & 8u) != 0u) {
            car[0x161] = static_cast<uint8_t>(car[0x161] | 4u);
            car[0xcd] = static_cast<uint8_t>(car[0xcd] | 4u);
            flags = logicFlags8(car[0xcd]);
            value = (value & 0xffffff00u) | 3u;
            car[0x84] = 3u;
        }
    }

    r->eax = value;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00440df7, FUN_00440df7, "regs(esi:ptr[0x24c]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
