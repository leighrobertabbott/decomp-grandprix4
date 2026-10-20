#include <gp4/gp4.h>
#include <gp4/x87.h>

// FUNCTION: GP4 0x0040fb47
// esi = a record whose byte +0x8d bit 0 disables the lookup (eax = 0, flags of the "test": all clear).
// Otherwise the low byte of eax is clamped to 1..[0x62ec6c] (signed, limit applied first), decremented,
// stored in DAT_006f2bf4, loaded with fild and interpolated through the curve table 0x62ecd8 by 0x411891
// (edx = 0x62ecd8 on return); the result is stored back with fistp into DAT_006f2bf4 and returned in eax.
// The flags are those the table lookup left.
void __cdecl FUN_0040fb47(gp4::Regs* r) {
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    int32_t count = static_cast<int32_t>(r->eax & 0xffu);

    if ((record[0x8d] & 1u) != 0u) {
        // test byte [esi+0x8d], 1 with the bit set: CF = OF = AF = ZF = SF = 0, PF = 0 (one bit)
        r->eax = 0;
        r->eflags = r->eflags & ~0x8d5u;
        return;
    }

    if (count > GP4_GLOBAL(int32_t, 0x0062ec6c)) {
        count = GP4_GLOBAL(int32_t, 0x0062ec6c);
    }
    if (count < 1) {
        count = 1;
    }
    count -= 1;
    GP4_GLOBAL(int32_t, 0x006f2bf4) = count;

    gp4::Regs callee = *r;
    callee.st_in = 1;
    callee.st_out = 1;
    callee.edx = 0x0062ecd8u;
    gp4::x87::set_in_st(callee, 0, gp4::x87::ext_i32(count));
    gp4::call_regs(0x00411891, callee);

    const int32_t value = gp4::x87::fistp32(gp4::x87::out_st(callee, 0));
    GP4_GLOBAL(int32_t, 0x006f2bf4) = value;

    r->eax = static_cast<uint32_t>(value);
    r->edx = 0x0062ecd8u;
    r->eflags = (r->eflags & ~0x8d5u) | (callee.eflags & 0x8d5u);
}
GP4_IMPL(0x0040fb47, FUN_0040fb47, "regs(eax:u32, esi:ptr[0x95]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=curves_0062ecb0")
