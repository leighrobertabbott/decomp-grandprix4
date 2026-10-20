#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: clears the 16 overlapping dwords at 0x00866015..0x00866024 (a counted loop of
// unaligned `mov dword [ecx+0x866014], 0`), selects mode 0xd in 0x0088ff30 and runs the mode
// dispatcher 0x005141b8 with the caller's registers (ecx is restored by push/pop around the loop; the
// loop's last `dec ecx` leaves ZF and the flags of 1 - 1, CF kept). popal restores every GPR; the
// flags are those the dispatcher leaves.
// FUNCTION: GP4 0x0051d160
void __cdecl FUN_0051d160(gp4::Regs* r) {
    for (uint32_t i = 0x10u; i != 0u; i--) {
        *(uint32_t*)(uintptr_t)(0x00866014u + i) = 0u;
    }
    GP4_GLOBAL(uint32_t, 0x0088ff30) = 0xdu;
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, r->eflags));
    gp4::call_regs(0x005141b8, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051d160, FUN_0051d160, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=layout=player_0040f8d9+disp_005141b8")
