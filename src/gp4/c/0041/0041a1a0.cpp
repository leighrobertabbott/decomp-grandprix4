#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 8-bit `cmp left, 0`.
static inline uint32_t cmpZeroFlags8(uint32_t left) {
    left &= 0xffu;
    return byteParity(left) | (left == 0u ? 0x40u : 0u) | (left & 0x80u);
}

// Flags of `shl value, 8` in the pinned oracle model: CF = last bit out, AF = 0, OF = bit31(value << 7 ^ result).
static inline uint32_t shl8Flags(uint32_t value) {
    const uint32_t result = value << 8;
    return ((value >> 24) & 1u)
        | byteParity(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((value << 7) ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

static inline void run(uint32_t address, gp4::Regs& regs) {
    regs.st_in = 0;
    regs.st_out = 0;
    gp4::call_regs(address, regs);
}

// Per-frame update of a car body (esi): steering/aero set-up passes (0x41a10d, 0x4168df, 0x416b4c,
// 0x417467), then the word +0xb4 (plus the word 0x006034d0 minus the word 0x00639642 when bit 2 of byte +0xca
// and the byte 0x006034d4 are set) is sign-extended and scaled by 256 into the dwords 0x006034b0 / 0x006034b4,
// and the force passes follow (0x416ec9 only when the byte 0x0062fb48 is non-zero). Registers and flags
// thread from one routine to the next.
// FUNCTION: GP4 0x0041a1a0
void __cdecl FUN_0041a1a0(gp4::Regs* r) {
    const uint8_t* car = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs c = *r;

    run(0x0041a10d, c);
    run(0x004168df, c);
    run(0x00416b4c, c);
    run(0x00417467, c);

    uint16_t word = *reinterpret_cast<const uint16_t*>(car + 0xb4);
    if ((car[0xca] & 4u) != 0u && GP4_GLOBAL(uint8_t, 0x006034d4) != 0u) {
        word = static_cast<uint16_t>(word + GP4_GLOBAL(uint16_t, 0x006034d0));
        word = static_cast<uint16_t>(word - GP4_GLOBAL(uint16_t, 0x00639642));
    }
    const uint32_t extended = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(word)));
    const uint32_t scaled = extended << 8;
    GP4_GLOBAL(uint32_t, 0x006034b0) = scaled;
    GP4_GLOBAL(uint32_t, 0x006034b4) = scaled;
    c.eax = scaled;
    c.eflags = (c.eflags & ~0x8d5u) | shl8Flags(extended);

    run(0x004168b6, c);
    run(0x00416b52, c);
    run(0x0041746d, c);
    run(0x0041a236, c);
    run(0x0041b5b6, c);
    run(0x004168b6, c);
    run(0x00416b52, c);
    run(0x0041746d, c);
    run(0x00417066, c);

    const uint32_t enabled = GP4_GLOBAL(uint8_t, 0x0062fb48);
    c.eflags = (c.eflags & ~0x8d5u) | cmpZeroFlags8(enabled);
    if (enabled != 0u) {
        run(0x00416ec9, c);
    }
    run(0x00417473, c);
    run(0x00419ccf, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x0041a1a0, FUN_0041a1a0, "regs(esi:ptr[0x390]:layout=rec_004132e5+rec_0041325b+rec_00412b88+rec_00419220+car_00416ec9, eax:u32, ebx:u32, ebp:u32, edx:u32) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=sums_0041a236+recs_006380d8+state_00416dec+state_0041a2ec+state_0041416e+state_00412b88+state_00419220+state_004193fa+state_0041931d+tyre_00601510+floor_00603498+state_00413397+wheel_004193fa+sel_0041b5b6+state_00416fce")
