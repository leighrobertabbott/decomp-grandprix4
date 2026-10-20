#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags left by a byte-wide TEST/OR: CF = OF = AF = 0, PF/ZF/SF from the result.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0040eb43
// When byte 0x7c2ea9 has its sign bit set it runs 0x0040e84e, 0x0040e914 and 0x0040ef41 on the esi
// record in turn, then marks the record (bit 6 of +0x88) when it is the car pointer in dword
// 0x679d2c. The flags are those of the last test, cmp or byte OR.
void __cdecl FUN_0040eb43(gp4::Regs* r) {
    const uint32_t self = r->esi;
    const uint8_t gate = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    uint32_t flags = logicFlags8(gate);                              // test byte [0x7c2ea9], 0xff

    if (static_cast<int8_t>(gate) < 0) {
        gp4::call_regs(0x0040e84e, *r);
        gp4::call_regs(0x0040e914, *r);
        gp4::call_regs(0x0040ef41, *r);

        const uint32_t current = GP4_GLOBAL(uint32_t, 0x00679d2c);
        flags = cmpFlags32(self, current);                           // cmp esi, [0x679d2c]
        if (self == current) {
            uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(self));
            GP4_FIELD(uint8_t, record, 0x88) |= 0x40;
            flags = logicFlags8(GP4_FIELD(uint8_t, record, 0x88));
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040eb43, FUN_0040eb43, "regs(eax:u32, esi:ptr[0x420]:layout=car_004129fa+rec_0040e84e) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0040e949+state_0040e84e+state_004327cd")
