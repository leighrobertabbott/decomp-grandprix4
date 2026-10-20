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

// Marks the cars that take part in a session: for each of the 22 car records at 0x0066fef0 (stride 0x41c),
// a car whose slot byte +0x20c is not 0xff, is set in the bit set at 0x007ab018 and has bit 7 of +0x7c gets
// bit 7 of +0x414 plus bit 6 (bit 6 of +0x7c clear: it also becomes the car [0x0062f8b0]) or bit 5 (bit 6 of
// +0x7c set); while the byte [0x007a8dfe] is 0xff it also clears bit 2 of +0xe5. Every other car has bits 7, 6
// and 5 of +0x414 cleared. eax, ecx and esi are saved/restored; the flags are those of the final
// `add esi, 0x41c`.
// FUNCTION: GP4 0x0044576b
void __cdecl FUN_0044576b(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x0062f8b0) = 0u;

    uint8_t* car = GP4_ARRAY(uint8_t, 0x0066fef0);
    uint32_t flags = 0;
    for (uint32_t remaining = 0x16; remaining != 0u; --remaining) {
        const uint32_t slot = car[0x20c];
        bool taking = false;
        if (slot != 0xffu) {
            const uint32_t word = GP4_ARRAY(uint32_t, 0x007ab018)[slot >> 5];
            if (((word >> (slot & 31u)) & 1u) != 0u && (car[0x7c] & 0x80u) != 0u) {
                taking = true;
            }
        }

        if (taking) {
            if (GP4_GLOBAL(uint8_t, 0x007a8dfe) == 0xffu) {
                car[0xe5] = static_cast<uint8_t>(car[0xe5] & 0xfbu);
            }
            if ((car[0x7c] & 0x40u) != 0u) {
                car[0x414] = static_cast<uint8_t>(car[0x414] | 0x80u);
                car[0x414] = static_cast<uint8_t>(car[0x414] | 0x20u);
            } else {
                car[0x414] = static_cast<uint8_t>(car[0x414] | 0x80u);
                car[0x414] = static_cast<uint8_t>(car[0x414] | 0x40u);
                GP4_GLOBAL(uint32_t, 0x0062f8b0) = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car));
            }
        } else {
            car[0x414] = static_cast<uint8_t>(car[0x414] & 0x7fu);
            car[0x414] = static_cast<uint8_t>(car[0x414] & 0xbfu);
            car[0x414] = static_cast<uint8_t>(car[0x414] & 0xdfu);
        }

        flags = addFlags32(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car)), 0x41cu);
        car += 0x41c;
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0044576b, FUN_0044576b, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carno_0066ff6c")
