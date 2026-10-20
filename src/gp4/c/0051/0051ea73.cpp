#include <gp4/gp4.h>
#include <gp4/flags.h>

// (a * b) >> 14 on the signed 64-bit product, low 32 bits kept (imul / shrd eax, edx, 14); the high
// half of the product is reported through *high (the original leaves it in edx).
static inline uint32_t term(int32_t a, int32_t b, uint32_t* high) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    *high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// FUNCTION: GP4 0x0051ea73
// Q14 matrix-vector product: the vector (0x008754c4, 0x008754c8, 0x008754cc) times the 3x3 block at
// 0x008752ac.. gives 0x008754ac / 0x008754b0 / 0x008754b4; each product is shifted right by 14 on its
// own and the three are summed. The original leaves the last product's high half in edx, the first
// two terms' sum in ebp, the result in eax and the flags of the final add.
void __cdecl FUN_0051ea73(gp4::Regs* r) {
    const int32_t v0 = GP4_GLOBAL(int32_t, 0x008754c4);
    const int32_t v1 = GP4_GLOBAL(int32_t, 0x008754c8);
    const int32_t v2 = GP4_GLOBAL(int32_t, 0x008754cc);
    uint32_t high;

    uint32_t t0 = term(v0, GP4_GLOBAL(int32_t, 0x008752b8), &high);
    uint32_t t1 = term(v1, GP4_GLOBAL(int32_t, 0x008752c4), &high);
    uint32_t partial = t0 + t1;
    uint32_t t2 = term(v2, GP4_GLOBAL(int32_t, 0x008752ac), &high);
    uint32_t sum = t2 + partial;
    GP4_GLOBAL(uint32_t, 0x008754ac) = sum;

    t0 = term(v0, GP4_GLOBAL(int32_t, 0x008752bc), &high);
    t1 = term(v1, GP4_GLOBAL(int32_t, 0x008752c8), &high);
    partial = t0 + t1;
    t2 = term(v2, GP4_GLOBAL(int32_t, 0x008752b0), &high);
    sum = t2 + partial;
    GP4_GLOBAL(uint32_t, 0x008754b0) = sum;

    t0 = term(v0, GP4_GLOBAL(int32_t, 0x008752c0), &high);
    t1 = term(v1, GP4_GLOBAL(int32_t, 0x008752cc), &high);
    partial = t0 + t1;
    t2 = term(v2, GP4_GLOBAL(int32_t, 0x008752b4), &high);
    sum = t2 + partial;
    GP4_GLOBAL(uint32_t, 0x008754b4) = sum;

    r->eax = sum;
    r->edx = high;
    r->ebp = partial;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(t2, partial));
}
GP4_IMPL(0x0051ea73, FUN_0051ea73, "regs() -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
