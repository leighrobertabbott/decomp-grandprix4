#include <gp4/gp4.h>
#include <gp4/flags.h>

// SetCurrentDirectoryA wrapper (thunk 0x005a48a6: jmp [0x5f0294]): eax is the directory string and
// returns the BOOL result. ebx/ecx/edx are saved and restored around the call. The flags are those
// of `cmp eax, 1`; when the result is not 1 `stc` additionally sets CF.
// FUNCTION: GP4 0x005368c3
void __cdecl FUN_005368c3(gp4::Regs* r) {
    const int result = gp4::stdcall_<0x005a48a6, int>(r->eax);
    uint32_t arith = gp4::flags::sub32(static_cast<uint32_t>(result), 1u);
    if (result != 1) {
        arith |= gp4::CF;
    }
    r->eax = static_cast<uint32_t>(result);
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005368c3, FUN_005368c3, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of)")
