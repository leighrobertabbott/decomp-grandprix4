#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Copies words of the record at esi into the corner table at 0x006380d8 (after FUN_00417688 has
// built its interpolation columns), picks four table entries by the clamped index (byte [esi+0x7c] & 0x3f) - 1,
// and stores two dword values (as doubles) into the 0x60-byte records at 0x006020d8 / 0x00602108.
// Leaves ebx = 0x006380d8, ebp from FUN_00417688, edx = clamped index, eax = byte [esi+0x202] * 0x60 and
// the flags of that IMUL.
// FUNCTION: GP4 0x00417774
void __cdecl FUN_00417774(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    GP4_GLOBAL(uint32_t, 0x00623c34) = (uint32_t)(int32_t)GP4_FIELD(const int16_t, rec, 0x62) << 8;
    GP4_GLOBAL(uint32_t, 0x006258c8) = (uint32_t)(int32_t)GP4_FIELD(const int16_t, rec, 0x20a) << 8;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00417688, c);

    uint8_t* table = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.ebx));
    GP4_FIELD(uint32_t, table, 0x40) = GP4_FIELD(const uint32_t, rec, 0x36c);
    GP4_FIELD(uint32_t, table, 0x340) = GP4_FIELD(const uint32_t, rec, 0x370);
    GP4_FIELD(uint32_t, table, 0x640) = GP4_FIELD(const uint32_t, rec, 0x374);
    GP4_FIELD(uint32_t, table, 0x940) = GP4_FIELD(const uint32_t, rec, 0x378);

    int32_t index = (int32_t)(GP4_FIELD(const uint8_t, rec, 0x7c) & 0x3f) - 1;
    if (index > 0x15) index = 0x15;
    GP4_FIELD(uint32_t, table, 0x178) = GP4_ARRAY(const uint32_t, 0x0066fb80)[index];
    GP4_FIELD(uint32_t, table, 0x478) = GP4_ARRAY(const uint32_t, 0x0066fbd8)[index];
    GP4_FIELD(uint32_t, table, 0x778) = GP4_ARRAY(const uint32_t, 0x0066fc30)[index];
    GP4_FIELD(uint32_t, table, 0xa78) = GP4_ARRAY(const uint32_t, 0x0066fc88)[index];

    const uint32_t offset = (uint32_t)GP4_FIELD(const uint8_t, rec, 0x202) * 0x60u;
    GP4_FIELD(double, GP4_ARRAY(uint8_t, 0x006020d8), offset) = (double)GP4_FIELD(const int32_t, rec, 0x228);
    GP4_FIELD(double, GP4_ARRAY(uint8_t, 0x00602108), offset) = (double)GP4_FIELD(const int32_t, rec, 0x22c);

    // flags of the final IMUL eax, eax, 0x60 (product of a byte and 0x60: no overflow; AF = 0)
    r->eflags = (r->eflags & ~0x8d5u) | parityFlag(offset) | (offset == 0u ? 0x40u : 0u);
    r->eax = offset;
    r->ebx = c.ebx;
    r->edx = (uint32_t)index;
    r->ebp = c.ebp;
}
GP4_IMPL(0x00417774, FUN_00417774, "regs(esi:ptr[0x380]:bytes) -> (eax, ebx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
