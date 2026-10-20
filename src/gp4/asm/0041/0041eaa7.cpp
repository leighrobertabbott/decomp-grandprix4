#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x0041eaa7
// eax = -round(record.dword[0x150] * DAT_006f2a04 * esi), the product kept at 80 bits on the x87
// stack; the rounded value is also left in DAT_006f2bf4 (esi is stored there and reloaded by FILD).
// The flags are those of "neg eax".
void __cdecl FUN_0041eaa7(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    const gp4::x87::Ext coefficient =
        gp4::x87::ext_i32(GP4_FIELD(int32_t, record, 0x150)) * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2a04));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = (int32_t)r->esi;
    const int32_t scaled =
        gp4::x87::fistp32(coefficient * gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4)));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = scaled;

    const uint32_t value = (uint32_t)GP4_GLOBAL(int32_t, 0x006f2bf4);
    const uint32_t result = 0u - value;
    const uint32_t flags = (value != 0u ? 1u : 0u) | parityFlag(result)
        | ((value ^ result) & 0x10u) | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | (value == 0x80000000u ? 0x800u : 0u);

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041eaa7, FUN_0041eaa7, "regs(edi:ptr[0x158]:bytes, esi:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=multiply_0040ce7b")
