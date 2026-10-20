#include <gp4/gp4.h>

// FUNCTION: GP4 0x00432a8b
void __cdecl FUN_00432a8b(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t self = r->esi;

    body[0x7b] = static_cast<uint8_t>(body[0x7b] & 0xf7u);
    body[0x7b] = static_cast<uint8_t>(body[0x7b] & 0xbfu);
    body[0x162] = static_cast<uint8_t>(body[0x162] & 0xfeu);
    body[0xfe] = static_cast<uint8_t>(body[0xfe] | 0x20u);
    body[0x7b] = static_cast<uint8_t>(body[0x7b] | 2u);
    body[0xce] = static_cast<uint8_t>(body[0xce] & 0xfeu);

    const uint32_t other = GP4_GLOBAL(uint32_t, 0x00679d2c);

    // lhs/rhs/result/flag kind for the instruction that leaves the final flags
    uint32_t lhs;
    uint32_t rhs;
    uint32_t res;
    bool isAdd;
    if (self == other) {
        lhs = GP4_GLOBAL(uint32_t, 0x0066d814);
        rhs = 0x7d0u;
        res = lhs + rhs;
        GP4_GLOBAL(uint32_t, 0x0066d848) = res;   // push/pop copy, then add 0x7d0
        isAdd = true;
    } else {
        lhs = self;
        rhs = other;
        res = lhs - rhs;                          // cmp esi, [0x679d2c]
        isAdd = false;
    }

    uint32_t f = 0u;
    if (isAdd) {
        if (res < lhs) f |= 0x1u;                                                  // CF
        if ((~(lhs ^ rhs) & (lhs ^ res) & 0x80000000u) != 0u) f |= 0x800u;         // OF
    } else {
        if (lhs < rhs) f |= 0x1u;                                                  // CF
        if (((lhs ^ rhs) & (lhs ^ res) & 0x80000000u) != 0u) f |= 0x800u;          // OF
    }
    uint32_t low = res & 0xffu;
    low ^= low >> 4;
    low ^= low >> 2;
    low ^= low >> 1;
    if ((low & 1u) == 0u) f |= 0x4u;                                               // PF
    if (((lhs ^ rhs ^ res) & 0x10u) != 0u) f |= 0x10u;                             // AF
    if (res == 0u) f |= 0x40u;                                                     // ZF
    if ((res & 0x80000000u) != 0u) f |= 0x80u;                                     // SF

    r->eflags = (r->eflags & ~0x8d5u) | f;
}
GP4_IMPL(0x00432a8b, FUN_00432a8b, "regs(esi:ptr[0x16a]:bytes) -> (cf, pf, af, zf, sf, of); globals=layout=state_00432a8b")
