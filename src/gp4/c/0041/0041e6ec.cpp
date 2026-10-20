#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit `sub left, right`.
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// pushal ... popal routine: builds the camera-relative vector (DAT_00623e30, 0x00623e34, 0x00623e38)
// from the two position dwords at esi and DAT_00637fe4 (scaled by 16), takes its dot product with
// 0x0040185a and publishes the result, DAT_00638010 - the dword at 0x006224b9 and DAT_00638014 in
// the record at ebx. Only the flags of the last "sub" escape; every register is restored.
// FUNCTION: GP4 0x0041e6ec
void __cdecl FUN_0041e6ec(gp4::Regs* r) {
    uint8_t* source = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* target = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    const uint32_t slot = r->ecx;

    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    gp4::call_regs(0x0041e9ab, callee);
    callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    gp4::call_regs(0x0041e9e7, callee);

    GP4_GLOBAL(uint32_t, 0x00623e30) = (GP4_GLOBAL(uint32_t, 0x00623e30) - GP4_FIELD(uint32_t, source, 0)) << 4;
    GP4_GLOBAL(uint32_t, 0x00623e38) = (GP4_GLOBAL(uint32_t, 0x00623e38) - GP4_FIELD(uint32_t, source, 4)) << 4;
    GP4_GLOBAL(uint32_t, 0x00623e34) = (GP4_GLOBAL(uint32_t, 0x00623e34) - GP4_GLOBAL(uint32_t, 0x00637fe4)) << 4;

    callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    gp4::call_regs(0x0040185a, callee);

    const uint32_t dot = GP4_GLOBAL(uint32_t, 0x00623e28);
    GP4_FIELD(uint32_t, source, 0x38c + slot * 4u) = dot;
    GP4_FIELD(uint32_t, target, 0x2c0) = dot;

    const uint32_t left = GP4_GLOBAL(uint32_t, 0x00638010);
    const uint32_t right = GP4_GLOBAL(uint32_t, 0x006224b9);
    GP4_FIELD(uint32_t, target, 0x168) = left - right;
    GP4_FIELD(uint32_t, target, 0x16c) = GP4_GLOBAL(uint32_t, 0x00638014);

    r->eflags = (r->eflags & ~0x8d5u) | subFlags32(left, right);
}
GP4_IMPL(0x0041e6ec, FUN_0041e6ec, "regs(ecx:u32[0..7], ebx:ptr[0x2d0]:bytes, esi:ptr[0x3c0]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
