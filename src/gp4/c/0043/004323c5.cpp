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

// Draws two bytes from the random generator 0x00401462 (b1, b2), and returns
// 0x00432372(clamp(DAT_00602b50, 0, 0x1900)) + b1 + b2 + 0x9bf in eax. edx is saved and restored
// (push/pop); the flags are those of the final add.
// FUNCTION: GP4 0x004323c5
void __cdecl FUN_004323c5(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;

    gp4::call_regs(0x00401462, call);
    const uint32_t first = call.eax & 0xffu;
    call.eax = first;
    gp4::call_regs(0x00401462, call);
    const uint32_t second = call.eax & 0xffu;
    const uint32_t offset = second + first - 0xffu + 0xabeu;

    int32_t limit = GP4_GLOBAL(int32_t, 0x00602b50);
    if (limit > 0x1900) {
        limit = 0x1900;
    }
    if (limit < 0) {
        limit = 0;
    }
    call.eax = static_cast<uint32_t>(limit);
    gp4::call_regs(0x00432372, call);

    r->eax = call.eax + offset;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(call.eax, offset);
}
GP4_IMPL(0x004323c5, FUN_004323c5, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0+state_00432372+temp_00602b50")
