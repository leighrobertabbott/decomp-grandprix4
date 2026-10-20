#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags left by a byte-wide AND: CF = OF = AF = 0, PF/ZF/SF from the result.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// Flags of the 8-bit `cmp left, right`.
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
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

// FUNCTION: GP4 0x00405f77
// al = byte +0x48 of the edi record & 3. Unless it is 1: sets bit 2 of byte +0xce and clears bit 5 of byte
// +0xff of the esi car, then 0x0051fb4f (mode byte 0x7c2ea9 -> 0..5) and, for the value 3, 0x00441a1b.
// The registers and flags are those the last executed instruction or callee left.
void __cdecl FUN_00405f77(gp4::Regs* r) {
    uint8_t* shape = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    const uint32_t kind = GP4_FIELD(uint8_t, shape, 0x48) & 3u;
    r->eax = (r->eax & 0xffffff00u) | kind;
    r->eflags = (r->eflags & ~0x8d5u) | cmpFlags8(kind, 1u);
    if (kind == 1u) {
        return;
    }

    GP4_FIELD(uint8_t, car, 0xce) |= 4;
    GP4_FIELD(uint8_t, car, 0xff) &= 0xdf;
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(GP4_FIELD(uint8_t, car, 0xff));

    gp4::call_regs(0x0051fb4f, *r);
    r->eflags = (r->eflags & ~0x8d5u) | cmpFlags32(r->eax, 3u);
    if (r->eax == 3u) {
        gp4::call_regs(0x00441a1b, *r);
    }
}
GP4_IMPL(0x00405f77, FUN_00405f77, "regs(eax:u32, esi:ptr[0x100]:bytes, edi:ptr[0x50]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=state_0040ee7b+tbl_0066d94c+state_00405f77")
