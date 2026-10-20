#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x4u : 0u;
}

// `test byte, imm`: CF = OF = AF = 0, PF/ZF/SF of the byte result.
static inline uint32_t flagsLogic8(uint8_t res) {
    uint32_t f = parityFlag(res);
    if (res == 0u) f |= 0x40u;
    if (res & 0x80u) f |= 0x80u;
    return f;
}

// Flags of a 16-bit CMP/SUB a - b.
static inline uint32_t flagsSub16(uint16_t a, uint16_t b) {
    const uint16_t res = (uint16_t)(a - b);
    uint32_t f = parityFlag(res);
    if (a < b) f |= 0x1u;
    f |= (uint32_t)((a ^ b ^ res) & 0x10u);
    if (res == 0u) f |= 0x40u;
    if (res & 0x8000u) f |= 0x80u;
    if (((a ^ b) & (a ^ res)) & 0x8000u) f |= 0x800u;
    return f;
}

static inline uint32_t setLow16(uint32_t reg, uint16_t v) {
    return (reg & 0xffff0000u) | v;
}

// edi = segment record, esi = car record. The `stc` tail sets CF and keeps the other flags of
// the last flag writer; after a callee the `jb` / `clc` pair leaves the callee's flags unchanged.
// FUNCTION: GP4 0x0042a300
void __cdecl FUN_0042a300(gp4::Regs* r) {
    const uint8_t* seg = (const uint8_t*)r->edi;
    uint8_t* car = (uint8_t*)r->esi;
    const uint8_t segFlags = seg[0x51];

    if (!(segFlags & 0x20u)) {
        r->eflags = (r->eflags & ~0x8d5u) | flagsLogic8((uint8_t)(segFlags & 0x20u)) | 1u;
        return;
    }
    if (segFlags & 0x80u) {
        r->eflags = (r->eflags & ~0x8d5u) | flagsLogic8((uint8_t)(segFlags & 0x80u)) | 1u;
        return;
    }

    const uint8_t dir = GP4_GLOBAL(uint8_t, 0x00604154);
    uint16_t ax = GP4_FIELD(uint16_t, car, 0x90);
    if (dir & 0x80u) ax = (uint16_t)(0u - ax);
    ax = (uint16_t)(ax + 0x200u);
    r->eax = setLow16(r->eax, ax);

    const int16_t limit = GP4_FIELD(int16_t, car, 0xb0);
    const uint32_t flags = flagsSub16(ax, (uint16_t)limit);
    if ((int16_t)ax >= limit) {
        r->eflags = (r->eflags & ~0x8d5u) | flags | 1u;
        return;
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;

    if (dir & 0x80u) {
        gp4::call_regs(0x0042ba87, *r);
    } else {
        gp4::Regs callee = *r;
        gp4::call_regs(0x0042bae4, callee);
        *r = callee;
    }
}
GP4_IMPL(0x0042a300, FUN_0042a300, "regs(edi:ptr[0x59]:bytes, esi:ptr[0x420]:layout=car_0042ba87, ebx:u32, eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800+state_004059ef+eq_0042ba87")
