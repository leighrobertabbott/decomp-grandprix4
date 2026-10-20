#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x00432654
void __cdecl FUN_00432654(gp4::Regs* r) {
    // pushal ... popal: every GPR is restored, only memory and the flags of the last flag
    // writer (the imul below) remain. The masked byte minus one is clamped above (signed) to
    // 21 only, so a zero byte gives row -1 (it reads the bytes in front of the 0x66d962 table).
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    int32_t row = static_cast<int32_t>(body[0x7c] & 0x3fu) - 1;
    if (row > 0x15) row = 0x15;
    int32_t column = GP4_ARRAY(const uint8_t, 0x0066d936)[row];
    if (column > 0x2b) column = 0x2b;
    const int32_t product = row * 0x2c;
    body[0x226] = GP4_ARRAY(const uint8_t, 0x0066d962)[column + product];
    GP4_FIELD(uint16_t, body, 0x224) = GP4_FIELD(uint16_t, body, 0x60);
    // imul eax, eax, 0x2c cannot overflow here: CF = OF = 0; AF = 0 in the pinned oracle model;
    // PF/ZF/SF follow the low product.
    const uint32_t result = static_cast<uint32_t>(product);
    const uint32_t flags = parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00432654, FUN_00432654, "regs(esi:ptr[0x22e]:bytes) -> (cf, pf, af, zf, sf, of); globals=layout=counter_0040f0dd")
