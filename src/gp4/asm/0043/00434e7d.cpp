#include <gp4/gp4.h>

// One "imul ; shrd eax, edx, N" step: low dword of the signed 64-bit product shifted right by N.
template <int N>
static inline uint32_t mulShr(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> N);
}

// High dword (edx) of the signed 64-bit product left by "imul".
static inline uint32_t mulHigh(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
}

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit "add dst, src" giving result.
static inline uint32_t addFlags32(uint32_t dst, uint32_t src, uint32_t result) {
    return (result < dst ? 0x001u : 0u)
        | parityFlag(result)
        | (((dst ^ src ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | (((~(dst ^ src) & (dst ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00434e7d
// Per-car update on the record at esi. A rate is built from the byte +0x227 (relative to the byte
// 0x007ad9f5) and the word +0xba with the factors 0x602a54..0x602a64; 0x4000 minus it is stored in
// 0x602a50. Unless +0x161 bit 0x40 is set or +0x7b bit 1 is clear, the word +0xee (scaled by +0x270
// when +0xe5 bit 0 is set, floored at 0x602a8c) is scaled by +0x62, 0x602a68, +0x182, 0x602a50 and
// the word 0x62d232/0x62d234 into an amount (else the amount is 0). amount * 0x603854 >> 12 is taken
// from the dword +0x19c; when that goes negative it is zeroed and, unless +0x8d bit 1 is set, +0x161
// bit 0x40 is set and 0x418097 (and 0x434d11 when +0xe5 bit 2 is set) is called. The word +0x60 is
// (+0x19c << 6) / 0x602a4c. Then 0x602a94 times the dword +0x244 is relaxed towards a target with
// the weight 0x602a7c, calling 0x440df7 after each step. When +0xe5 bit 2 is set, |(+0xe - +0xec) *
// 2 * +0x9a| >> 8 is added to one dword of the 0x66ec60 table (row by the car number +0x7c, column
// by the byte table 0x66d936), saturating to 0x7fffffff on carry.
void __cdecl FUN_00434e7d(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs c = *r;

    uint32_t ebp = r->ebp;
    uint32_t amount;
    if ((car[0x161] & 0x40u) != 0u || (car[0x7b] & 2u) == 0u) {
        amount = 0u;
    } else {
        const int32_t delta = static_cast<int32_t>(car[0x227]) - static_cast<int32_t>(GP4_GLOBAL(uint8_t, 0x007ad9f5));
        uint32_t sum = mulShr<14>(delta, GP4_GLOBAL(int32_t, 0x00602a54));
        const int32_t word0ba = GP4_FIELD(uint16_t, car, 0xba);
        if ((car[0xe5] & 1u) == 0u) {
            sum += mulShr<14>(word0ba, GP4_GLOBAL(int32_t, 0x00602a58));
        } else {
            sum += mulShr<14>(word0ba, GP4_GLOBAL(int32_t, 0x00602a5c));
            if ((car[0xe6] & 8u) == 0u) {
                sum += GP4_GLOBAL(uint32_t, 0x00602a60);
                if ((car[0x78] & 0x10u) == 0u) {
                    sum += GP4_GLOBAL(uint32_t, 0x00602a64);
                }
            }
        }
        ebp = sum;
        GP4_GLOBAL(uint32_t, 0x00602a50) = 0x4000u - sum;

        int32_t value = GP4_FIELD(uint16_t, car, 0xee);
        if ((car[0xe5] & 1u) != 0u) {
            value = static_cast<int32_t>(mulShr<14>(value, GP4_FIELD(uint16_t, car, 0x270)));
        }
        if (value < GP4_GLOBAL(int32_t, 0x00602a8c)) {
            value = GP4_GLOBAL(int32_t, 0x00602a8c);
        }
        value = static_cast<int32_t>(mulShr<14>(value, GP4_FIELD(uint16_t, car, 0x62)));
        value = static_cast<int32_t>(mulShr<14>(value, GP4_GLOBAL(int32_t, 0x00602a68)));
        value = static_cast<int32_t>(mulShr<14>(value, GP4_FIELD(uint16_t, car, 0x182)));
        value = static_cast<int32_t>(mulShr<14>(value, GP4_GLOBAL(int32_t, 0x00602a50)));
        const int32_t select = ((car[0xe5] & 1u) == 0u) ? GP4_GLOBAL(uint16_t, 0x0062d234) : GP4_GLOBAL(uint16_t, 0x0062d232);
        amount = mulShr<14>(value, select);
    }

    const int32_t step = GP4_GLOBAL(int32_t, 0x00603854);
    const uint32_t used = mulShr<12>(static_cast<int32_t>(amount), step);
    const int32_t left = static_cast<int32_t>(GP4_FIELD(uint32_t, car, 0x19c) - used);
    GP4_FIELD(int32_t, car, 0x19c) = left;
    if (left < 0) {
        GP4_FIELD(int32_t, car, 0x19c) = 0;
        if ((car[0x8d] & 2u) == 0u) {
            car[0x161] = static_cast<uint8_t>(car[0x161] | 0x40u);
            c.eax = used;
            c.edx = mulHigh(static_cast<int32_t>(amount), step);
            c.ebp = ebp;
            gp4::call_regs(0x00418097, c);
            if ((car[0xe5] & 4u) != 0u) {
                gp4::call_regs(0x00434d11, c);
            }
        }
    }

    uint32_t rem;
    const uint32_t level = gp4::div64(static_cast<uint64_t>(GP4_FIELD(uint32_t, car, 0x19c)) << 6,
                                      GP4_GLOBAL(uint32_t, 0x00602a4c), &rem);
    GP4_FIELD(uint16_t, car, 0x60) = static_cast<uint16_t>(level);

    const int32_t scaled = static_cast<int32_t>(mulShr<12>(static_cast<int32_t>(amount), 0xa00));
    uint32_t eax = mulShr<6>(scaled, GP4_GLOBAL(int32_t, 0x00602a78));
    uint32_t edx = mulHigh(scaled, GP4_GLOBAL(int32_t, 0x00602a78));
    GP4_GLOBAL(uint32_t, 0x00602a90) = eax;

    for (uint32_t n = GP4_GLOBAL(uint32_t, 0x00602a94); n != 0u; --n) {
        int32_t speed = GP4_FIELD(int16_t, car, 0x9a);
        if (speed < GP4_GLOBAL(int32_t, 0x00602a70)) {
            speed = GP4_GLOBAL(int32_t, 0x00602a70);
        }
        const int32_t current = GP4_FIELD(int32_t, car, 0x244);
        uint32_t target = mulShr<14>(static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00602a80) - static_cast<uint32_t>(current)), speed);
        target = mulShr<14>(static_cast<int32_t>(target), GP4_GLOBAL(int32_t, 0x00602a88));
        target += GP4_GLOBAL(uint32_t, 0x00602a90);
        if ((car[0x7b] & 2u) != 0u) {
            target += GP4_GLOBAL(uint32_t, 0x00602a74);
        }
        target = mulShr<10>(static_cast<int32_t>(target), 0xa00);
        target += GP4_GLOBAL(uint32_t, 0x00602a84);
        const uint32_t pulled = mulShr<14>(static_cast<int32_t>(target), GP4_GLOBAL(int32_t, 0x00602a7c));
        const uint32_t kept = mulShr<14>(static_cast<int32_t>(0x4000u - GP4_GLOBAL(uint32_t, 0x00602a7c)), current);
        const uint32_t next = kept + pulled;
        GP4_FIELD(uint32_t, car, 0x244) = next;

        c.eax = next;
        c.ecx = n;
        c.edx = pulled;
        c.ebp = ebp;
        gp4::call_regs(0x00440df7, c);
        eax = c.eax;
        edx = pulled;
    }

    uint32_t flags;
    if ((car[0xe5] & 4u) != 0u) {
        const int16_t diff = static_cast<int16_t>(GP4_FIELD(uint16_t, car, 0xe) - GP4_FIELD(uint16_t, car, 0xec));
        const int32_t doubled = static_cast<int32_t>(static_cast<uint32_t>(static_cast<int32_t>(diff)) << 1);
        const int32_t speed = GP4_FIELD(int16_t, car, 0x9a);
        const int64_t product = static_cast<int64_t>(doubled) * static_cast<int64_t>(speed);
        uint32_t wear = static_cast<uint32_t>(static_cast<uint64_t>(product));
        edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
        if (static_cast<int32_t>(wear) < 0) {
            wear = 0u - wear;
        }
        wear >>= 8;
        ebp = wear;
        eax = wear;

        int32_t row = static_cast<int32_t>(car[0x7c] & 0x3fu) - 1;
        if (row > 0x15) {
            row = 0x15;
        }
        const uint32_t column = GP4_ARRAY(uint8_t, 0x0066d936)[row];
        uint32_t* slot = GP4_ARRAY(uint32_t, 0x0066ec60) + row * 0x2c + static_cast<int32_t>(column);
        const uint32_t old = *slot;
        const uint32_t total = old + wear;
        *slot = total;
        flags = addFlags32(old, wear, total);
        if (total < old) {
            *slot = 0x7fffffffu;
        }
    } else {
        flags = 0x044u;   // test byte [esi+0xe5], 4 with a zero result: ZF, PF
    }

    r->eax = eax;
    r->ecx = 0u;
    r->edx = edx;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00434e7d, FUN_00434e7d, "regs(esi:ptr[0x278]:bytes, ecx:u32, ebp:u32) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=counter_0040f0dd+state_00434e7d")
