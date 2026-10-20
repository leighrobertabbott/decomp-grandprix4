#include <gp4/gp4.h>
#include <gp4/x87.h>

using gp4::x87::Ext;
using gp4::x87::ext;

struct Unknown_00416789 {
    uint8_t Unknown00[0x2bc];
    int32_t Unknown2BC[4];
};

// For each of the four slots (ecx = 0..3, ebx walking the 0x300-byte records from 0x6380d8 to
// 0x638cd8) clamps the dword at +0x2bc + ecx*4 to 1..0xfa000, parks it in DAT_006f2bf4 and calls
// 0x00411a22 with -(value * DAT_00601440) on the x87 stack.
// pushal/popal restore every register; the flags left are those of the final
// "cmp ebx, 0x638cd8" with ebx == 0x638cd8 (zero result: ZF | PF).
// FUNCTION: GP4 0x00416789
void __cdecl FUN_00416789(gp4::Regs* r) {
    const Unknown_00416789* body = reinterpret_cast<const Unknown_00416789*>(r->esi);

    uint32_t slot = 0;
    uint8_t* record = GP4_ARRAY(uint8_t, 0x006380d8);
    do {
        int32_t value = body->Unknown2BC[slot];
        if (value > 0xfa000) {
            value = 0xfa000;
        }
        if (value < 1) {
            value = 1;
        }
        GP4_GLOBAL(int32_t, 0x006f2bf4) = value;

        const Ext x = gp4::x87::fchs(gp4::x87::ext_i32(value) * ext(GP4_GLOBAL(double, 0x00601440)));

        gp4::Regs call = *r;
        call.ebx = (uint32_t)(uintptr_t)record;
        call.ecx = slot;
        call.st_in = 1;
        call.st_out = 0;
        call.st_xin_mask = 0;
        call.st_xout_mask = 0;
        gp4::x87::set_in_st(call, 0, x);
        gp4::call_regs(0x00411a22, call);

        ++slot;
        record += 0x300;
    } while (record != GP4_ARRAY(uint8_t, 0x00638cd8));

    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00416789, FUN_00416789, "regs(esi:ptr[0x2d0]:bytes) -> (cf, pf, af, zf, sf, of); globals=keep")
