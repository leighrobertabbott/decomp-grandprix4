#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of a byte AND: CF = OF = AF = 0, ZF/SF/PF from the byte result.
static inline uint32_t logicFlags8(uint8_t value) {
    return parityFlag8(value)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x80u) != 0u ? 0x80u : 0u);
}

// Flags of a 32-bit ADD.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// imul + shrd eax, edx, shift.
static inline uint32_t mulShift(int32_t a, int32_t b, int shift) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> shift);
}

// FUNCTION: GP4 0x0042595c
void __cdecl FUN_0042595c(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t input = r->eax;
    uint32_t edx = r->edx;
    uint32_t ebp = r->ebp;

    GP4_GLOBAL(uint32_t, 0x00602aa0) = input;

    if (car[0x70] >= 2u && (car[0x8d] & 0x02u) == 0u) {
        if ((car[0x179] & 0x01u) != 0u) {
            // test byte [esi+0x179], 1 with a nonzero result: all six flags clear.
            r->eflags &= ~0x8d5u;
            return;
        }

        gp4::Regs sub = *r;
        ebp = input;
        edx = static_cast<uint32_t>(car[0x415]) >> 1;
        bool proceed = true;
        if (static_cast<int32_t>(edx) < 4) {
            edx = edx * 4u + 0x10u;
            gp4::call_regs(0x00401462, sub);
            const uint32_t roll = sub.eax & 0xffu;
            if (static_cast<int32_t>(edx) >= static_cast<int32_t>(roll)) proceed = false;
        }

        if (proceed) {
            gp4::call_regs(0x0042593d, sub);
            const uint32_t chance = sub.eax;

            const uint32_t scale = GP4_GLOBAL(uint16_t, 0x0062d228);
            const uint32_t term = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, car, 0xb6)))
                                * GP4_GLOBAL(uint32_t, 0x006223e4) + 0x4000u;
            const uint32_t scaled = mulShift(static_cast<int32_t>(term), static_cast<int32_t>(scale), 0xe);
            const uint32_t factor = static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x007ad8a2)) * 3u + 200u;
            uint32_t remainder;
            const uint32_t quotient = gp4::div64(static_cast<uint64_t>(factor) * scaled, 500u, &remainder);
            const uint32_t level = GP4_GLOBAL(uint16_t, 0x007ad89e);
            const uint32_t threshold = mulShift(static_cast<int32_t>(quotient),
                                                static_cast<int32_t>(GP4_ARRAY(uint16_t, 0x00602ae8)[level]), 0xe);
            edx = threshold;
            ebp = 500u;

            if (static_cast<int32_t>(chance) < static_cast<int32_t>(threshold)) {
                car[0x179] |= 0x01u;
                car[0x1f5] = 4u;

                gp4::call_regs(0x0042593d, sub);
                const int32_t low = GP4_GLOBAL(int32_t, 0x00602b44);
                const int32_t high = GP4_GLOBAL(int32_t, 0x00602b48);
                car[0x41b] = static_cast<uint8_t>(mulShift(static_cast<int32_t>(sub.eax), high - low, 0xe)
                                                  + static_cast<uint32_t>(low));
                car[0x203] &= 0xdfu;

                gp4::call_regs(0x00401462, sub);
                if (static_cast<int32_t>(sub.eax & 0xffu) <= GP4_GLOBAL(int32_t, 0x00601f00)) {
                    int32_t excess = static_cast<int32_t>(GP4_FIELD(int16_t, car, 0x10e)) - 0x3000;
                    if (excess < 0) excess = 0;
                    if (excess > 0x1000) excess = 0x1000;
                    const int32_t limit = ((0x1000 - excess) + 0x800) << 1;
                    gp4::call_regs(0x00401462, sub);
                    if (static_cast<int32_t>((sub.eax & 0xffu) << 6) <= limit) car[0x203] |= 0x20u;
                }

                gp4::call_regs(0x00401462, sub);
                int32_t level1f4 = static_cast<int32_t>(
                    mulShift(static_cast<int32_t>((sub.eax & 0xffu) << 6), GP4_GLOBAL(int32_t, 0x00601ee8), 0xe)
                    + GP4_GLOBAL(uint32_t, 0x00601eec));
                if (level1f4 > 0x4000) level1f4 = 0x4000;
                car[0x1f4] = static_cast<uint8_t>(static_cast<uint32_t>(level1f4) >> 8);

                gp4::call_regs(0x0042593d, sub);
                const uint32_t base = GP4_GLOBAL(uint16_t, 0x0062d224);
                const uint32_t top = GP4_GLOBAL(uint16_t, 0x0062d226);
                uint32_t result = mulShift(static_cast<int32_t>(sub.eax), static_cast<int32_t>(top - base), 0xe) + base;

                gp4::call_regs(0x00401462, sub);
                const uint32_t level2 = GP4_GLOBAL(uint16_t, 0x007ad89e);
                if (static_cast<int32_t>(sub.eax & 0xffu) < GP4_ARRAY(int32_t, 0x00602af4)[level2]) {
                    car[0x230] |= 0x40u;
                    gp4::call_regs(0x00401462, sub);
                    const uint32_t x = mulShift(static_cast<int32_t>((sub.eax & 0xffu) << 6),
                                                GP4_ARRAY(int32_t, 0x00602b08)[level2], 0xe)
                                     + static_cast<uint32_t>(GP4_ARRAY(int32_t, 0x00602b1c)[level2]);
                    result = mulShift(static_cast<int32_t>(x),
                                      static_cast<int32_t>(GP4_GLOBAL(uint16_t, 0x0062d24e)), 8);
                }

                const uint32_t offset = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, car, 0x1ac)));
                r->eflags = (r->eflags & ~0x8d5u) | addFlags32(result, offset);
                r->eax = result + offset;
                r->ecx = level2;
                r->edx = offset;
                r->ebp = 500u;
                return;
            }
        }
    }

    car[0x179] &= 0xfeu;
    const uint8_t flags230 = static_cast<uint8_t>(car[0x230] & 0xbfu);
    car[0x230] = flags230;
    r->eax = input;
    r->edx = edx;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(flags230);
}
GP4_IMPL(0x0042595c, FUN_0042595c, "regs(eax:u32, ecx:u32, edx:u32, ebp:u32, esi:ptr[0x423]:layout=rec_0042595c) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_0042595c")
