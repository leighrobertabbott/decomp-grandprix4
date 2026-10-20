#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of the 32-bit "neg value".
static inline uint32_t negFlags32(uint32_t value, uint32_t result) {
    return (value != 0u ? 1u : 0u)
        | parityFlag(result)
        | (((value ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (value == 0x80000000u ? 0x800u : 0u);
}

// Dword +0x30 of the 0x300-byte body at `first` and of the next body.
static __declspec(noinline) int32_t pairedField30(uint8_t* first) {
    return GP4_FIELD(int32_t, first, 0x30) + GP4_FIELD(int32_t, first, 0x330);
}

// FUNCTION: GP4 0x00416add
// Runs the body update 0x004169c3 on the first two 0x300-byte bodies at 0x006380d8 (ecx = 0, 1) and
// stores -((first.+0x30 + second.+0x30) >> 1) at 0x00601ab4. On exit eax is that value, ebx the
// first body, ecx = 1, edx / ebp the last update's products; the flags are those of the NEG.
void __cdecl FUN_00416add(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;

    call.ebx = 0x006380d8u;
    call.ecx = 0u;
    gp4::call_regs(0x004169c3, call);

    call.ebx = 0x006383d8u;
    call.ecx = 1u;
    gp4::call_regs(0x004169c3, call);

    const uint32_t half = (uint32_t)(pairedField30(GP4_ARRAY(uint8_t, 0x006380d8)) >> 1);
    const uint32_t result = 0u - half;
    GP4_GLOBAL(uint32_t, 0x00601ab4) = result;

    r->eax = result;
    r->ebx = 0x006380d8u;
    r->ecx = 1u;
    r->edx = call.edx;
    r->ebp = call.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | negFlags32(half, result);
}
GP4_IMPL(0x00416add, FUN_00416add, "regs() -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=floor_00603498+wheel_006380d8")
