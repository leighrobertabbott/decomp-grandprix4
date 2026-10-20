#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of `shl value, 5` under the oracle model: CF = last bit shifted out, AF = 0,
// OF = bit31((value << 4) ^ result), ZF/SF/PF from the result.
static inline uint32_t shl5Flags32(uint32_t value) {
    const uint32_t result = value << 5;
    return ((value >> 27) & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((value << 4) ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Debris object (ebx): computes the mass properties of the part mask [ebx+0x4c] with 0x00444517 (and, when bit 1
// of byte +0x6f is set, rescales them to the run-time car with 0x00444715), then copies the centroid (<< 5) to
// +0x30/+0x34/+0x38, the inertias to +0x3c/+0x40/+0x44, the mass to +0x48 and the launch coefficient
// [0x62a080] to +0x50/+0x54. pushal/popal keep every register; the flags are those of the last `shl eax, 5`.
// FUNCTION: GP4 0x00444837
void __cdecl FUN_00444837(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    uint8_t* object = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.ebx));
    c.eax = GP4_FIELD(uint32_t, object, 0x4c);
    gp4::call_regs(0x00444517, c);

    object = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.ebx));
    if ((GP4_FIELD(uint8_t, object, 0x6f) & 2u) != 0u) {
        gp4::call_regs(0x00444715, c);
        object = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.ebx));
    }

    GP4_FIELD(uint32_t, object, 0x30) = GP4_GLOBAL(uint32_t, 0x0062a064) << 5;
    GP4_FIELD(uint32_t, object, 0x34) = GP4_GLOBAL(uint32_t, 0x0062a068) << 5;
    const uint32_t centroidZ = GP4_GLOBAL(uint32_t, 0x0062a06c);
    GP4_FIELD(uint32_t, object, 0x38) = centroidZ << 5;
    GP4_FIELD(uint32_t, object, 0x3c) = GP4_GLOBAL(uint32_t, 0x0062a070);
    GP4_FIELD(uint32_t, object, 0x40) = GP4_GLOBAL(uint32_t, 0x0062a074);
    GP4_FIELD(uint32_t, object, 0x44) = GP4_GLOBAL(uint32_t, 0x0062a078);
    const uint32_t launch = GP4_GLOBAL(uint32_t, 0x0062a080);
    GP4_FIELD(uint32_t, object, 0x54) = launch;
    GP4_FIELD(uint32_t, object, 0x50) = launch;
    GP4_FIELD(uint32_t, object, 0x48) = GP4_GLOBAL(uint32_t, 0x0062a07c);

    r->eflags = (r->eflags & ~0x8d5u) | shl5Flags32(centroidZ);
}
GP4_IMPL(0x00444837, FUN_00444837, "regs(ebx:ptr[0x80]:layout=rec_00424878+mask_00444837) -> (cf, pf, af, zf, sf, of); globals=layout=state_00444517+state_00424a21")
