#include <gp4/gp4.h>
#include <gp4/x87.h>

struct Unknown_006380d8 {
    uint8_t Unknown00[0xcc];
    int32_t UnknownCC;      // result
    uint8_t UnknownD0[0x24c - 0xd0];
    int32_t Unknown24C;
    int32_t Unknown250;
    int32_t Unknown254;
    uint8_t Unknown258[0x300 - 0x258];
};

// For each of the four 0x300-byte records at 0x6380d8:
//   t = (DAT_006228d0 * K) * rec.Unknown254 - (DAT_006228d8 * K) * rec.Unknown24C   (K = qword 0x6f2bc4)
// evaluated on the x87 stack (80-bit), rounded by fistp into DAT_00623e18, then
//   rec.UnknownCC = (DAT_00623e18 >> 4) + DAT_00637f78.
// eax holds the last record's result, ebx is reset to 0x6380d8 and esi is saved and restored;
// the flags are those of the final "cmp ebx, 0x638cd8" (equal: ZF and PF set).
// FUNCTION: GP4 0x0041f2f1
void __cdecl FUN_0041f2f1(gp4::Regs* r) {
    using gp4::x87::Ext;
    uint32_t result = 0;
    Unknown_006380d8* record = GP4_ARRAY(Unknown_006380d8, 0x006380d8);
    do {
        const Ext scaleA = gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006228d0)) * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2bc4));
        const Ext first = scaleA * gp4::x87::ext_i32(record->Unknown254);
        const Ext scaleB = gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006228d8)) * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2bc4));
        const Ext second = scaleB * gp4::x87::ext_i32(record->Unknown24C);
        GP4_GLOBAL(int32_t, 0x00623e18) = gp4::x87::fistp32(first - second);
        result = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00623e18) >> 4) + GP4_GLOBAL(uint32_t, 0x00637f78);
        record->UnknownCC = static_cast<int32_t>(result);
        ++record;
    } while (reinterpret_cast<uintptr_t>(record) != 0x00638cd8u);

    r->eax = result;
    r->ebx = 0x006380d8u;
    // cmp ebx, 0x638cd8 with equal operands: ZF and PF set, everything else clear
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0041f2f1, FUN_0041f2f1, "regs() -> (eax, ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=spin_0041f28d")
