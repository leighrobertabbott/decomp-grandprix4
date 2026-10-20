#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

static inline uint32_t widthMask(uint32_t bits) {
    return bits == 32u ? 0xffffffffu : ((1u << bits) - 1u);
}

static inline uint32_t resultFlags(uint32_t result, uint32_t bits, bool cf, bool af, bool of) {
    result &= widthMask(bits);
    return parityFlag(result)
        | (cf ? 0x01u : 0u)
        | (af ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | (((result >> (bits - 1u)) & 1u) != 0u ? 0x80u : 0u)
        | (of ? 0x800u : 0u);
}

// flags of "cmp a, b" / "sub a, b" / "neg b" (a = 0)
static inline uint32_t subFlags(uint32_t a, uint32_t b, uint32_t bits) {
    const uint32_t mask = widthMask(bits);
    a &= mask;
    b &= mask;
    const uint32_t r = (a - b) & mask;
    return resultFlags(r, bits, a < b, ((a ^ b ^ r) & 0x10u) != 0u,
                       ((((a ^ b) & (a ^ r)) >> (bits - 1u)) & 1u) != 0u);
}

// flags of "add a, b"
static inline uint32_t addFlags(uint32_t a, uint32_t b, uint32_t bits) {
    const uint32_t mask = widthMask(bits);
    a &= mask;
    b &= mask;
    const uint32_t r = (a + b) & mask;
    return resultFlags(r, bits, r < a, ((a ^ b ^ r) & 0x10u) != 0u,
                       (((~(a ^ b) & (a ^ r)) >> (bits - 1u)) & 1u) != 0u);
}

// flags of a "test byte, imm" (CF = OF = AF = 0)
static inline uint32_t testFlags8(uint32_t value) {
    return resultFlags(value, 8u, false, false, false);
}

// FUNCTION: GP4 0x0042b040
void __cdecl FUN_0042b040(gp4::Regs* r) {
    const uint8_t* other = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->ebx));
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t eax = r->eax;
    uint32_t ecx = r->ecx;
    uint32_t edx = r->edx;
    uint32_t flags = 0;

    do {
        const uint32_t gate = GP4_GLOBAL(uint8_t, 0x00603e6e);
        flags = testFlags8(gate);
        if (gate != 0u) break;
        const uint32_t recordBits = record[0x9c] & 2u;
        flags = testFlags8(recordBits);
        if (recordBits != 0u) break;
        const uint32_t otherBits = other[0x9c] & 2u;
        flags = testFlags8(otherBits);
        if (otherBits != 0u) break;

        int16_t ax = GP4_GLOBAL(int16_t, 0x00603d9c);
        const int16_t cx = GP4_GLOBAL(int16_t, 0x00604402);
        eax = (eax & 0xffff0000u) | static_cast<uint16_t>(ax);
        ecx = (ecx & 0xffff0000u) | static_cast<uint16_t>(cx);
        flags = subFlags(static_cast<uint16_t>(cx), static_cast<uint16_t>(ax), 16u);

        const int16_t speed = *reinterpret_cast<const int16_t*>(other + 0x9a);
        if (cx < ax) {
            int16_t dx = *reinterpret_cast<const int16_t*>(other + 0x11e);
            edx = (edx & 0xffff0000u) | static_cast<uint16_t>(dx);
            flags = subFlags(static_cast<uint16_t>(ax), static_cast<uint16_t>(dx), 16u);
            if (ax <= dx) break;
            flags = subFlags(static_cast<uint16_t>(speed), 0x755u, 16u);
            if (speed < 0x755) break;
            const int16_t before = dx;
            dx = static_cast<int16_t>(static_cast<uint16_t>(dx) + 0x100u);
            edx = (edx & 0xffff0000u) | static_cast<uint16_t>(dx);
            flags = addFlags(static_cast<uint16_t>(before), 0x100u, 16u);
            flags = subFlags(static_cast<uint16_t>(ax), static_cast<uint16_t>(dx), 16u);
            if (ax > dx) ax = dx;
        } else {
            int16_t dx = *reinterpret_cast<const int16_t*>(other + 0x11c);
            flags = subFlags(0u, static_cast<uint16_t>(dx), 16u);
            dx = static_cast<int16_t>(0u - static_cast<uint16_t>(dx));
            edx = (edx & 0xffff0000u) | static_cast<uint16_t>(dx);
            flags = subFlags(static_cast<uint16_t>(ax), static_cast<uint16_t>(dx), 16u);
            if (ax >= dx) break;
            flags = subFlags(static_cast<uint16_t>(speed), 0x755u, 16u);
            if (static_cast<uint16_t>(speed) < 0x755u) break;
            const int16_t before = dx;
            dx = static_cast<int16_t>(static_cast<uint16_t>(dx) - 0x100u);
            edx = (edx & 0xffff0000u) | static_cast<uint16_t>(dx);
            flags = subFlags(static_cast<uint16_t>(before), 0x100u, 16u);
            flags = subFlags(static_cast<uint16_t>(ax), static_cast<uint16_t>(dx), 16u);
            if (ax < dx) ax = dx;
        }

        GP4_GLOBAL(int16_t, 0x00603d9c) = ax;
        flags = subFlags(0u, static_cast<uint16_t>(ax), 16u);
        const uint16_t negated = static_cast<uint16_t>(0u - static_cast<uint16_t>(ax));
        const uint16_t base = GP4_GLOBAL(uint16_t, 0x00604402);
        const uint16_t sum = static_cast<uint16_t>(negated + base);
        flags = addFlags(negated, base, 16u);
        GP4_GLOBAL(uint16_t, 0x00604404) = sum;
        eax = (eax & 0xffff0000u) | sum;
    } while (false);

    r->eax = eax;
    r->ecx = ecx;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042b040, FUN_0042b040,
         "regs(eax:u32, ecx:u32, edx:u32, ebx:ptr[0x128]:bytes, esi:ptr[0xa8]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
