#include <gp4/gp4.h>
#include <gp4/x87.h>

struct UnknownPoint_004228df {
    uint32_t Unknown00;
    int32_t Unknown04;
    int32_t Unknown08;
    uint32_t Unknown0C;
    uint32_t Unknown10;
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of `left + right` whose result is `result`.
static inline uint32_t addFlags(uint32_t left, uint32_t right, uint32_t result) {
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((((left ^ result) & (right ^ result)) & 0x80000000u) >> 20);
}

// Rotates the offset (DAT_006082c8, 0) by the angle eax * DAT_006f2bdc: stores
// round(cos * radius) at DAT_006082d0 and round(sin * radius) at DAT_006082cc, then copies the
// source point at ebx to the destination at edi (+0x0, +0x4 plus the cosine term, +0x8 plus the
// sine term, +0x10), advancing esi by one and edi by 0x14. eax leaves holding the source +0x10;
// the flags are those of the final "add edi, 0x14".
// FUNCTION: GP4 0x004228df
void __cdecl FUN_004228df(gp4::Regs* r) {
    const UnknownPoint_004228df* source = (const UnknownPoint_004228df*)r->ebx;
    UnknownPoint_004228df* dest = (UnknownPoint_004228df*)r->edi;
    const int32_t angle = static_cast<int32_t>(r->eax);

    GP4_GLOBAL(int32_t, 0x006f2bf4) = angle;
    const gp4::x87::Ext scaled = gp4::x87::ext_i32(angle) * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2bdc));
    gp4::x87::Ext sine;
    gp4::x87::Ext cosine;
    gp4::x87::fsincos(scaled, &sine, &cosine);

    GP4_GLOBAL(int32_t, 0x006082d0) =
        gp4::x87::fistp32(cosine * gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006082c8)));
    GP4_GLOBAL(int32_t, 0x006082cc) =
        gp4::x87::fistp32(sine * gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006082c8)));

    dest->Unknown00 = source->Unknown00;
    dest->Unknown04 = source->Unknown04 + GP4_GLOBAL(int32_t, 0x006082d0);
    dest->Unknown08 = source->Unknown08 + GP4_GLOBAL(int32_t, 0x006082cc);
    dest->Unknown10 = source->Unknown10;

    const uint32_t newDest = r->edi + 0x14u;
    r->eax = source->Unknown10;
    r->esi = r->esi + 1u;
    r->edi = newDest;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(newDest - 0x14u, 0x14u, newDest);
}
GP4_IMPL(0x004228df, FUN_004228df, "regs(eax:i32, ebx:ptr[0x40]:bytes, edi:ptr[0x40]:bytes, esi:u32) -> (eax, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
