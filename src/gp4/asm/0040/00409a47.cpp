#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Takes the +0x418 word of the record in esi (scaled by DAT_00603568 >> 14 unless bit 0 of +0xe5 is
// set), clamps its magnitude to DAT_006013ac (unsigned), rounds it through DAT_0062ee80 with an
// FILD/FMUL/FISTP via DAT_006f2bf4, then returns (value * DAT_00603468) >> 14 in eax. edx is saved and
// restored; the flags are those of the final "shrd eax, edx, 14".
// FUNCTION: GP4 0x00409a47
void __cdecl FUN_00409a47(gp4::Regs* r) {
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t recordFlags = record[0xe5];
    int32_t value = static_cast<int32_t>(*reinterpret_cast<const int16_t*>(record + 0x418));

    if ((recordFlags & 1u) == 0u) {
        const int64_t product = static_cast<int64_t>(value) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603568));
        value = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14));
    }

    uint32_t magnitude = static_cast<uint32_t>(value);
    if (value < 0) {
        magnitude = 0u - magnitude;
        if (!(magnitude < GP4_GLOBAL(uint32_t, 0x006013ac))) {
            magnitude = GP4_GLOBAL(uint32_t, 0x006013ac);
        }
        magnitude = 0u - magnitude;
    } else if (!(magnitude < GP4_GLOBAL(uint32_t, 0x006013ac))) {
        magnitude = GP4_GLOBAL(uint32_t, 0x006013ac);
    }

    GP4_GLOBAL(uint32_t, 0x006f2bf4) = magnitude;
    const gp4::x87::Ext scaled = gp4::x87::ext_i32(static_cast<int32_t>(magnitude))
        * gp4::x87::ext(GP4_GLOBAL(double, 0x0062ee80));
    const int32_t rounded = gp4::x87::fistp32(scaled);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = rounded;

    const int64_t product = static_cast<int64_t>(rounded) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603468));
    const uint64_t wide = static_cast<uint64_t>(product);
    const uint32_t previous = static_cast<uint32_t>(wide >> 13);
    const uint32_t result = static_cast<uint32_t>(wide >> 14);

    const uint32_t flags = (previous & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((previous ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00409a47, FUN_00409a47, "regs(esi:ptr[0x420]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
