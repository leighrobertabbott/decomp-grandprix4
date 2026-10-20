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

// FUNCTION: GP4 0x0042785a
// `pushal ; test byte [esi+0x230], 0x10 ; je run ; popal ; ret`. With bit 4 of +0x230 clear it runs
// 0x004036ca, stores the word 0x004275d1 returns at +0x142 and sets bit 3 of the byte at +0xfe. Every
// register is restored; the flags are those of the last test or byte OR.
void __cdecl FUN_0042785a(gp4::Regs* r) {
    gp4::Regs saved = *r;
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(saved.esi));
    uint32_t flags;

    const uint32_t masked = GP4_FIELD(uint8_t, record, 0x230) & 0x10u;
    if (masked != 0u) {
        flags = logicFlags8(masked);
    } else {
        gp4::Regs first = saved;
        gp4::call_regs(0x004036ca, first);
        gp4::Regs second = saved;
        gp4::call_regs(0x004275d1, second);
        GP4_FIELD(uint16_t, record, 0x142) = static_cast<uint16_t>(second.eax);
        GP4_FIELD(uint8_t, record, 0xfe) |= 8;
        flags = logicFlags8(GP4_FIELD(uint8_t, record, 0xfe));
    }

    saved.eflags = (saved.eflags & ~0x8d5u) | flags;
    *r = saved;
}
GP4_IMPL(0x0042785a, FUN_0042785a, "regs(eax:u32, edi:ptr[0x1c4]:bytes, esi:ptr[0x420]:layout=car_004129fa+rec_00409416) -> (cf, pf, af, zf, sf, of); globals=layout=state_00403800+state_0042785a")
