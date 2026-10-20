#include <gp4/gp4.h>
#include <gp4/x87.h>

// FUNCTION: GP4 0x0040fb95
// Without bit 0 of the byte at +0x8d of the record in esi: the index al, limited to [1, DAT_0062ec6c]
// and reduced by one (kept in DAT_006f2bf4), is run through the two curve tables 0x0062ecd8 and
// 0x0062ecb0 (FUN_00411891) and the first result divided by the second is rounded back into
// DAT_006f2bf4 and eax; edx is left at 0x0062ecb0 and the flags come from the second table call.
// With the bit set: eax = DAT_0062ec9c, flags of "test byte, 1" (all clear), edx untouched.
void __cdecl FUN_0040fb95(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    if ((GP4_FIELD(uint8_t, body, 0x8d) & 1u) != 0u) {
        r->eax = GP4_GLOBAL(uint32_t, 0x0062ec9c);
        r->eflags = r->eflags & ~0x8d5u;
        return;
    }

    int32_t index = static_cast<int32_t>(r->eax & 0xffu);
    if (index > GP4_GLOBAL(int32_t, 0x0062ec6c)) {
        index = GP4_GLOBAL(int32_t, 0x0062ec6c);
    }
    if (index < 1) {
        index = 1;
    }
    --index;
    GP4_GLOBAL(int32_t, 0x006f2bf4) = index;

    const gp4::x87::Ext x = gp4::x87::ext_i32(index);

    gp4::Regs first = *r;
    first.st_in = 1;
    first.st_out = 1;
    first.edx = 0x0062ecd8u;
    gp4::x87::set_in_st(first, 0, x);
    gp4::call_regs(0x00411891, first);
    const gp4::x87::Ext numerator = gp4::x87::out_st(first, 0);

    gp4::Regs second = *r;
    second.st_in = 1;
    second.st_out = 1;
    second.edx = 0x0062ecb0u;
    gp4::x87::set_in_st(second, 0, x);
    gp4::call_regs(0x00411891, second);
    const gp4::x87::Ext denominator = gp4::x87::out_st(second, 0);

    const int32_t rounded = gp4::x87::fistp32(numerator / denominator);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = rounded;

    r->eax = static_cast<uint32_t>(rounded);
    r->edx = 0x0062ecb0u;
    r->eflags = (r->eflags & ~0x8d5u) | (second.eflags & 0x8d5u);
}
GP4_IMPL(0x0040fb95, FUN_0040fb95, "regs(eax:u32, edx:u32, esi:ptr[0x98]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=curves_0062ecb0")
