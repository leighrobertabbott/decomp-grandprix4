#include <gp4/gp4.h>
#include <gp4/flags.h>

// `cmp byte [0x00852c38], 0`: when the byte is zero, runs the ring-record walker 0x005594a5 (its registers
// and flags come back); otherwise returns with the flags of the compare.
// FUNCTION: GP4 0x00559560
void __cdecl FUN_00559560(gp4::Regs* r) {
    const uint32_t flag = GP4_GLOBAL(uint8_t, 0x00852c38);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(flag, 0u));
    if (flag == 0u) {
        gp4::call_regs(0x005594a5, *r);
    }
}
GP4_IMPL(0x00559560, FUN_00559560, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:u32, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ringf_005596fd")
