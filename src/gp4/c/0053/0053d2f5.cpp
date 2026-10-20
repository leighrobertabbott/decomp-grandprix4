#include <gp4/gp4.h>
#include <gp4/flags.h>

// Register-block thunk through the function pointer [0x00a69878]: loads esi/edi/eax/ebx/ecx/edx/ebp from the dwords
// 0x00a6987c .. 0x00a69894 (in that order), calls the pointer with them, and stores the seven registers back to the
// same dwords. pushal/popal keep the caller's registers; the flags are those the callee leaves.
// FUNCTION: GP4 0x0053d2f5
void __cdecl FUN_0053d2f5(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.esi = GP4_GLOBAL(uint32_t, 0x00a6987c);
    call.edi = GP4_GLOBAL(uint32_t, 0x00a69880);
    call.eax = GP4_GLOBAL(uint32_t, 0x00a69884);
    call.ebx = GP4_GLOBAL(uint32_t, 0x00a69888);
    call.ecx = GP4_GLOBAL(uint32_t, 0x00a6988c);
    call.edx = GP4_GLOBAL(uint32_t, 0x00a69890);
    call.ebp = GP4_GLOBAL(uint32_t, 0x00a69894);
    gp4::call_regs(GP4_GLOBAL(uint32_t, 0x00a69878), call);
    GP4_GLOBAL(uint32_t, 0x00a6987c) = call.esi;
    GP4_GLOBAL(uint32_t, 0x00a69880) = call.edi;
    GP4_GLOBAL(uint32_t, 0x00a69884) = call.eax;
    GP4_GLOBAL(uint32_t, 0x00a69888) = call.ebx;
    GP4_GLOBAL(uint32_t, 0x00a6988c) = call.ecx;
    GP4_GLOBAL(uint32_t, 0x00a69890) = call.edx;
    GP4_GLOBAL(uint32_t, 0x00a69894) = call.ebp;
    r->eflags = gp4::flags::merge(r->eflags, call.eflags);
}
GP4_IMPL(0x0053d2f5, FUN_0053d2f5, "regs(cf:u32[0..1], pf:u32[0..1], af:u32[0..1], zf:u32[0..1], sf:u32[0..1], of:u32[0..1]) -> (cf, pf, af, zf, sf, of); globals=layout=fnptr_0053d2f5")
