#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of the 32-bit `left - right` (SUB / CMP).
static inline uint32_t subtractFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Arithmetic flags of a logic result (TEST / OR / AND): CF = OF = AF = 0.
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

static const uint32_t kArithFlags = 0x8d5u;

// esi = the 3-dword position record (x at +0, z at +8, already in the 2^-11 record frame x 2^11).
// 0x40460a rotates the position relative to the record at edi into 0x934df0 / 0x934df4; when the
// (absolute) rotated x, scaled down by 8 bits and minus 0x200, is below the record's span word
// (+0x60 for negative x, +0x62 otherwise) the walk 0x405c9a (dot-product ring walk) is used, otherwise
// the byte 0x603c78 is set to the sign of x (0 / 0xff) and the walk 0x405a12 (side test) is used.
// ebx and ecx are preserved; edi, eax, edx and the flags are those left by the chosen walk.
// FUNCTION: GP4 0x00405c30
void __cdecl FUN_00405c30(gp4::Regs* r) {
    const uint32_t* position = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t posX = position[0];
    const uint32_t posZ = position[2];

    gp4::Regs c = *r;
    GP4_GLOBAL(uint32_t, 0x00623e30) = posX;
    GP4_GLOBAL(uint32_t, 0x00623e38) = posZ;
    gp4::call_regs(0x0040460a, c);

    const uint32_t rotatedX = GP4_GLOBAL(uint32_t, 0x00934df0);
    uint32_t magnitude = rotatedX;
    uint32_t span;
    if (static_cast<int32_t>(rotatedX) < 0) {
        magnitude = 0u - rotatedX;
        span = GP4_FIELD(uint16_t, reinterpret_cast<void*>(static_cast<uintptr_t>(c.edi)), 0x60);
    } else {
        span = GP4_FIELD(uint16_t, reinterpret_cast<void*>(static_cast<uintptr_t>(c.edi)), 0x62);
    }
    const uint32_t limit = static_cast<uint32_t>(static_cast<int32_t>(magnitude) >> 8) - 0x200u;

    c.edx = span;
    c.ecx = posX;
    c.ebx = posZ;
    if (static_cast<int32_t>(limit) < static_cast<int32_t>(span)) {
        c.eax = limit;
        c.eflags = (c.eflags & ~kArithFlags) | subtractFlags32(limit, span);
        gp4::call_regs(0x00405c9a, c);
    } else {
        c.eax = limit;
        c.eflags = (c.eflags & ~kArithFlags) | subtractFlags32(limit, span);
        GP4_GLOBAL(uint8_t, 0x00603c78) = 0;
        c.eflags = (c.eflags & ~kArithFlags) | logicFlags32(rotatedX);
        if (static_cast<int32_t>(rotatedX) < 0) {
            GP4_GLOBAL(uint8_t, 0x00603c78) = 0xff;
        }
        gp4::call_regs(0x00405a12, c);
    }

    r->eax = c.eax;
    r->edx = c.edx;
    r->edi = c.edi;
    r->eflags = (r->eflags & ~kArithFlags) | (c.eflags & kArithFlags);
}
GP4_IMPL(0x00405c30, FUN_00405c30, "regs(esi:ptr[0x10]:i32[-0x4000000..0x3fff7ff], edi:rec[0x8943a0,0x160,0..11]) -> (eax, edx, edi, cf, pf, af, zf, sf, of); globals=layout=ring_00405a12")
