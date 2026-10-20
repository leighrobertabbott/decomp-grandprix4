#include <gp4/gp4.h>

static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (result < left ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x004293e4
// eax = ((k1 * x^2) >> 14) + ((sqrt(|w * x|) * k2) >> 14), with x = max(word [esi+0x174], DAT_00601ba0)
// (signed compare), w = signed word [esi+0x9a], k1 = DAT_00601b9c, k2 = DAT_00601b98 and the
// products taken as signed 64-bit values whose bits 14..45 are kept (imul / shrd 14).
// ecx and edx are preserved; flags are those of the final add.
void __cdecl FUN_004293e4(gp4::Regs* r) {
    uint8_t* body = (uint8_t*)r->esi;

    int32_t x = (int32_t)GP4_FIELD(uint16_t, body, 0x174);
    if (x < GP4_GLOBAL(int32_t, 0x00601ba0)) {
        x = GP4_GLOBAL(int32_t, 0x00601ba0);
    }

    // imul ecx (eax = ecx): low half of x*x, then imul by DAT_00601b9c, shrd eax, edx, 14
    const int32_t square = (int32_t)((int64_t)x * x);
    const int64_t scaled = (int64_t)square * GP4_GLOBAL(int32_t, 0x00601b9c);
    const uint32_t quadratic = (uint32_t)((uint64_t)scaled >> 14);

    // movsx eax, word [esi+0x9a]; imul ecx; abs
    const int32_t w = (int32_t)(int16_t)GP4_FIELD(uint16_t, body, 0x9a);
    int32_t product = (int32_t)((int64_t)w * x);
    if (product < 0) {
        product = (int32_t)(0u - (uint32_t)product);
    }

    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    callee.eax = (uint32_t)product;
    gp4::call_regs(0x004010f3, callee);       // integer square root

    const int64_t root = (int64_t)(int32_t)callee.eax * GP4_GLOBAL(int32_t, 0x00601b98);
    const uint32_t linear = (uint32_t)((uint64_t)root >> 14);

    r->eax = linear + quadratic;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(linear, quadratic);
}
GP4_IMPL(0x004293e4, FUN_004293e4, "regs(esi:ptr[0x17c]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
