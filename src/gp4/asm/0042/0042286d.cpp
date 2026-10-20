#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags (CF PF AF ZF SF OF) of the 32-bit "add left, right".
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ result) & (right ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// eax = angle (2^31/pi units). Rotates the vector (DAT_006082c8, 0) by that angle into DAT_006082d0 =
// round(r cos) and DAT_006082cc = round(r sin), then fills a 0x28-byte record at edi (two 5-dword
// halves) from the 5-dword point at ebx: x -/+ DAT_006082c4 at +0/+0x14, y + round(r cos) at
// +4/+0x18, z + round(r sin) at +8/+0x1c, and dword +0x10 at +0x10/+0x24. Advances esi by 2 and edi
// by 0x28; the flags are those of the final "add edi, 0x28"; eax ends as the dword at ebx+0x10.
// FUNCTION: GP4 0x0042286d
void __cdecl FUN_0042286d(gp4::Regs* r) {
    const int32_t angle = static_cast<int32_t>(r->eax);
    uint8_t* source = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    GP4_GLOBAL(int32_t, 0x006f2bf4) = angle;
    const gp4::x87::Ext theta = gp4::x87::ext_i32(angle) * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2bdc));
    gp4::x87::Ext sine;
    gp4::x87::Ext cosine;
    gp4::x87::fsincos(theta, &sine, &cosine);

    GP4_GLOBAL(int32_t, 0x006082d0) = gp4::x87::fistp32(cosine * gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006082c8)));
    GP4_GLOBAL(int32_t, 0x006082cc) = gp4::x87::fistp32(sine * gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006082c8)));

    const uint32_t half = GP4_GLOBAL(uint32_t, 0x006082c4);
    GP4_FIELD(uint32_t, record, 0x00) = GP4_FIELD(uint32_t, source, 0x00) - half;
    GP4_FIELD(uint32_t, record, 0x14) = GP4_FIELD(uint32_t, source, 0x00) + half;

    const uint32_t y = GP4_FIELD(uint32_t, source, 0x04) + GP4_GLOBAL(uint32_t, 0x006082d0);
    GP4_FIELD(uint32_t, record, 0x04) = y;
    GP4_FIELD(uint32_t, record, 0x18) = y;

    const uint32_t z = GP4_FIELD(uint32_t, source, 0x08) + GP4_GLOBAL(uint32_t, 0x006082cc);
    GP4_FIELD(uint32_t, record, 0x08) = z;
    GP4_FIELD(uint32_t, record, 0x1c) = z;

    const uint32_t extra = GP4_FIELD(uint32_t, source, 0x10);
    GP4_FIELD(uint32_t, record, 0x10) = extra;
    GP4_FIELD(uint32_t, record, 0x24) = extra;

    r->eax = extra;
    r->esi += 2u;
    r->edi += 0x28u;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(static_cast<uint32_t>(r->edi - 0x28u), 0x28u);
}
GP4_IMPL(0x0042286d, FUN_0042286d, "regs(eax:i32, ebx:ptr[0x18]:bytes, edi:ptr[0x30]:bytes, esi:u32) -> (eax, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
