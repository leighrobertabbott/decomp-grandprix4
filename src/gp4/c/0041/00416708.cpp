#include <gp4/gp4.h>
#include <gp4/x87.h>

// FUNCTION: GP4 0x00416708
// For each of the four 0x300-byte records at 0x006380d8 (ebx walks to 0x00638cd8, ecx = 0..3) hands
// -(dword [0x603020 + ecx*4] * qword [0x6f29c4]) on the x87 stack (80-bit) to 0x00411922. All GPRs are
// preserved by the pushal/popal pair; the flags are those of the loop's last `cmp ebx, 0x638cd8`
// (equal: ZF = PF = 1).
void __cdecl FUN_00416708(gp4::Regs* r) {
    uint32_t index = 0;
    for (uint32_t walker = 0x006380d8u; walker != 0x00638cd8u; walker += 0x300u) {
        // fild dword [ecx*4 + 0x603020] / fmul qword [0x6f29c4] / fchs
        const gp4::x87::Ext x = gp4::x87::fchs(
            gp4::x87::ext_i32(GP4_ARRAY(int32_t, 0x00603020)[index]) * gp4::x87::ext(GP4_GLOBAL(double, 0x006f29c4)));

        gp4::Regs call = *r;
        call.st_in = 1;
        call.st_out = 0;
        call.ebx = walker;
        call.ecx = index;
        gp4::x87::set_in_st(call, 0, x);
        gp4::call_regs(0x00411922, call);
        ++index;
    }

    // last executed flag writer: cmp ebx, 0x638cd8 with ebx equal -> ZF = PF = 1
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00416708, FUN_00416708, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00416708")
