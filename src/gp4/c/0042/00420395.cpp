#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of add left, right
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (result < left ? 1u : 0u);
}

// Converts the integers DAT_006228f4/f8/fc to doubles at +0xb8/+0xc0/+0xc8 of the body at
// 0x00622e98, rotates them with 0x00401f02 (results at +0xa0/+0xa8/+0xb0), rounds the results
// into DAT_00622bdc/be0/be4 with 0x004022b5 and stores each plus the matching pair of
// DAT_006228dc/e0/e4 and DAT_00601d44/48/4c into DAT_006228d0/d4/d8. Leaves edx = the body address,
// eax = the last sum and the flags of its final add.
// FUNCTION: GP4 0x00420395
void __cdecl FUN_00420395(gp4::Regs* r) {
    uint8_t* body = GP4_ARRAY(uint8_t, 0x00622e98);

    GP4_FIELD(double, body, 0xb8) = static_cast<double>(GP4_GLOBAL(int32_t, 0x006228f4));
    GP4_FIELD(double, body, 0xc0) = static_cast<double>(GP4_GLOBAL(int32_t, 0x006228f8));
    GP4_FIELD(double, body, 0xc8) = static_cast<double>(GP4_GLOBAL(int32_t, 0x006228fc));

    r->edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(body));
    gp4::call_regs(0x00401f02, *r);
    gp4::call_regs(0x004022b5, *r);

    GP4_GLOBAL(uint32_t, 0x006228d0) =
        GP4_GLOBAL(uint32_t, 0x00622bdc) + GP4_GLOBAL(uint32_t, 0x006228dc) + GP4_GLOBAL(uint32_t, 0x00601d44);
    GP4_GLOBAL(uint32_t, 0x006228d4) =
        GP4_GLOBAL(uint32_t, 0x00622be0) + GP4_GLOBAL(uint32_t, 0x006228e0) + GP4_GLOBAL(uint32_t, 0x00601d48);

    const uint32_t partial = GP4_GLOBAL(uint32_t, 0x00622be4) + GP4_GLOBAL(uint32_t, 0x006228e4);
    const uint32_t last = GP4_GLOBAL(uint32_t, 0x00601d4c);
    const uint32_t sum = partial + last;
    GP4_GLOBAL(uint32_t, 0x006228d8) = sum;

    r->eax = sum;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(partial, last);
}
GP4_IMPL(0x00420395, FUN_00420395, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
