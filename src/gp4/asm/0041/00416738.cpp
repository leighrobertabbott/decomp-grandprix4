#include <gp4/gp4.h>
#include <gp4/x87.h>

// FUNCTION: GP4 0x00416738
// For each of the four records (ebx walks 0x006380d8 .. 0x00638cd8 in steps of 0x300, ecx = 0..3)
// takes the dword [esi + 0x2ec + ecx*4], clamps it to 1..0xfa000 (kept in DAT_006f2bf4), scales it
// by -DAT_00601440 in 80-bit precision and hands that on the x87 stack to 0x00411a22 with ecx = the
// index. All GPRs are preserved by the pushal/popal pair; the flags are those of the loop's last
// `cmp ebx, 0x638cd8` (equal).
void __cdecl FUN_00416738(gp4::Regs* r) {
    uint8_t* owner = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    uint32_t index = 0;
    for (uint32_t walker = 0x006380d8u; walker != 0x00638cd8u; walker += 0x300u) {
        int32_t value = GP4_FIELD(int32_t, owner, 0x2ec + index * 4u);
        if (value > 0xfa000) {
            value = 0xfa000;
        }
        if (value < 1) {
            value = 1;
        }
        GP4_GLOBAL(int32_t, 0x006f2bf4) = value;

        // fild / fmul qword [0x601440] / fchs
        const gp4::x87::Ext x = gp4::x87::fchs(gp4::x87::ext_i32(value) * gp4::x87::ext(GP4_GLOBAL(double, 0x00601440)));

        gp4::Regs call = *r;
        call.st_in = 1;
        call.st_out = 0;
        call.ebx = walker;
        call.ecx = index;
        gp4::x87::set_in_st(call, 0, x);
        gp4::call_regs(0x00411a22, call);
        ++index;
    }

    // last executed flag writer: cmp ebx, 0x638cd8 with ebx equal -> ZF = PF = 1
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00416738, FUN_00416738, "regs(esi:ptr[0x304]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
