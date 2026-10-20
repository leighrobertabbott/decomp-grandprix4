#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of "cmp left, right" / "sub".
static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (left < right ? 1u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of "test value, -1": CF = OF = AF = 0.
static inline uint32_t testFlags32(uint32_t value) {
    return parityFlag(value)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x80000000u) != 0u ? 0x80u : 0u);
}

// "imul r/m32" (edx:eax = signed product) followed by "shrd eax, edx, count":
// bits count..count+31 of the 64-bit product.
static inline uint32_t mulShr(int32_t a, int32_t b, unsigned count) {
    const uint64_t product = static_cast<uint64_t>(static_cast<int64_t>(a) * static_cast<int64_t>(b));
    return static_cast<uint32_t>(product >> count);
}

// FUNCTION: GP4 0x00404aa3
void __cdecl FUN_00404aa3(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const int32_t selector = GP4_GLOBAL(int32_t, 0x006030d0);
    uint32_t flags;

    if (selector == 0) {
        GP4_GLOBAL(int32_t, 0x00603104) = 0;
        GP4_GLOBAL(int32_t, 0x00603108) = 0;
        GP4_GLOBAL(int32_t, 0x0060310c) = 0;
        GP4_GLOBAL(int32_t, 0x00603110) = 0;
        flags = testFlags32(0u);
    } else if (selector < 0) {
        const int32_t half0 = GP4_GLOBAL(int32_t, 0x006013a0) >> 1;
        const uint32_t first = mulShr(half0, selector, 14);
        GP4_GLOBAL(int32_t, 0x00603104) = static_cast<int32_t>(first);
        GP4_GLOBAL(int32_t, 0x00603108) = static_cast<int32_t>(first);

        const int32_t half1 = GP4_GLOBAL(int32_t, 0x006013a4) >> 1;
        const int64_t product = static_cast<int64_t>(half1) * static_cast<int64_t>(selector);
        const uint64_t bits = static_cast<uint64_t>(product);
        const uint32_t second = static_cast<uint32_t>(bits >> 14);
        GP4_GLOBAL(int32_t, 0x0060310c) = static_cast<int32_t>(second);
        GP4_GLOBAL(int32_t, 0x00603110) = static_cast<int32_t>(second);

        // Flags come from the final "shrd eax, edx, 14" (pinned oracle model: CF = bit 13 of the
        // product, AF = 0, OF = bit31(previous ^ result)).
        const uint32_t previous = static_cast<uint32_t>(bits >> 13);
        flags = static_cast<uint32_t>((bits >> 13) & 1u)
            | parityFlag(second)
            | (second == 0u ? 0x40u : 0u)
            | ((second >> 31) << 7)
            | (((previous ^ second) >> 31) << 11);
        r->eax = second;
        r->edx = static_cast<uint32_t>(bits >> 32);
    } else if (selector < GP4_GLOBAL(int32_t, 0x00624790)) {
        GP4_GLOBAL(int32_t, 0x00603104) = 0;
        GP4_GLOBAL(int32_t, 0x00603108) = 0;
        GP4_GLOBAL(int32_t, 0x0060310c) = 0;
        GP4_GLOBAL(int32_t, 0x00603110) = 0;
        flags = subtractionFlags32(static_cast<uint32_t>(selector),
                                   static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00624790)));
        r->eax = static_cast<uint32_t>(selector);
    } else {
        const int32_t angle = GP4_GLOBAL(int32_t, 0x006030b4);
        const int32_t entry = GP4_ARRAY(int32_t, 0x00639654)[angle >> 14];
        const uint32_t s1 = mulShr(entry, GP4_FIELD(uint16_t, body, 0xd6), 14);
        const uint32_t s2 = mulShr(static_cast<int32_t>(s1), GP4_FIELD(uint16_t, body, 0xb2), 4);
        const uint32_t sum = mulShr(static_cast<int32_t>(s2), GP4_GLOBAL(int32_t, 0x0060313c), 14);
        GP4_GLOBAL(int32_t, 0x00603104) = static_cast<int32_t>(sum);
        GP4_GLOBAL(int32_t, 0x00603108) = static_cast<int32_t>(sum);

        const uint32_t d1 = mulShr(angle, GP4_GLOBAL(int32_t, 0x006030bc), 0x1c);
        const uint32_t drag = mulShr(static_cast<int32_t>(d1), GP4_GLOBAL(int32_t, 0x0062f23c), 14);
        const uint32_t negDrag = 0u - drag;

        int32_t plus = static_cast<int32_t>(drag + static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00603104)));
        if (plus < 0) plus = 0;
        GP4_GLOBAL(int32_t, 0x00603104) = plus;

        int32_t minus = static_cast<int32_t>(negDrag + static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00603108)));
        flags = subtractionFlags32(static_cast<uint32_t>(minus), 0u);
        if (minus < 0) minus = 0;
        GP4_GLOBAL(int32_t, 0x00603108) = minus;

        GP4_GLOBAL(int32_t, 0x0060310c) = 0;
        GP4_GLOBAL(int32_t, 0x00603110) = 0;
        r->eax = static_cast<uint32_t>(plus);
        r->edx = static_cast<uint32_t>(minus);
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00404aa3, FUN_00404aa3, "regs(eax:u32, edx:u32, esi:ptr[0xe0]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
