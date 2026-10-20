#include <gp4/gp4.h>
#include <gp4/x87.h>

struct Unknown_006380d8 {
    uint8_t Unknown00[0xc4];
    int32_t UnknownC4;      // result
    uint8_t UnknownC8[0x24c - 0xc8];
    int32_t Unknown24C;
    int32_t Unknown250;
    int32_t Unknown254;
    uint8_t Unknown258[0x300 - 0x258];
};
static_assert(sizeof(Unknown_006380d8) == 0x300, "record size");

// For each of the four 0x300-byte records at 0x6380d8:
//   t = (DAT_00623eb8 * K) * rec.Unknown254 - (DAT_00623ec0 * K) * rec.Unknown24C   (K = qword 0x6f2bcc)
// evaluated on the x87 stack (80-bit), rounded by fistp into DAT_00623e18, then
//   rec.UnknownC4 = DAT_00623e18 + (DAT_00638050 >> 4).
// eax holds DAT_00638050 >> 4, edx the last record's result, ebx is reset to 0x6380d8 and esi is
// saved and restored; the flags are those of the final "cmp ebx, 0x638cd8" (equal: ZF and PF set).
// FUNCTION: GP4 0x0041f28d
void __cdecl FUN_0041f28d(gp4::Regs* r) {
    using gp4::x87::Ext;
    uint32_t result = 0;
    uint32_t shifted = 0;
    Unknown_006380d8* record = GP4_ARRAY(Unknown_006380d8, 0x006380d8);
    do {
        const Ext scaleA = gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x00623eb8)) * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2bcc));
        const Ext first = scaleA * gp4::x87::ext_i32(record->Unknown254);
        const Ext scaleB = gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x00623ec0)) * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2bcc));
        const Ext second = scaleB * gp4::x87::ext_i32(record->Unknown24C);
        GP4_GLOBAL(int32_t, 0x00623e18) = gp4::x87::fistp32(first - second);
        shifted = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00638050) >> 4);
        result = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00623e18)) + shifted;
        record->UnknownC4 = static_cast<int32_t>(result);
        ++record;
    } while (reinterpret_cast<uintptr_t>(record) != 0x00638cd8u);

    r->eax = shifted;
    r->edx = result;
    r->ebx = 0x006380d8u;
    // cmp ebx, 0x638cd8 with equal operands: ZF and PF set, everything else clear
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0041f28d, FUN_0041f28d, "regs() -> (eax, edx, ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=spin_0041f28d")
