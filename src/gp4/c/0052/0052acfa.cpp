#include <gp4/gp4.h>
#include <gp4/flags.h>

// Address of the SYSTEMTIME buffer; kept out of line so the buffer base is the only global the field reads see
// (the original indexes it through esi).
static __declspec(noinline) const uint16_t* systemTime(uint32_t address) {
    return reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(address));
}

// pushal wrapper: reads the local time into the SYSTEMTIME buffer 0x0081d37c (GetLocalTime through the thunk
// 0x005a493c; ebx, ecx and edx are saved around the call), loads ecx = wYear, eax = wMonth, edx = wDay with its
// second byte replaced by the low byte of the month (`mov dh, al`), and stores the date word that 0x0052affd
// computes from them into [0x0082459c]. popal restores every GPR; the flags are those of 0x0052affd.
// FUNCTION: GP4 0x0052acfa
void __cdecl FUN_0052acfa(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    const uint32_t savedEbx = c.ebx;
    const uint32_t savedEcx = c.ecx;
    const uint32_t savedEdx = c.edx;
    const uint32_t args[1] = {0x0081d37cu};
    gp4::call_regs_stdcall(0x005a493c, c, args);
    c.ebx = savedEbx;
    c.ecx = savedEcx;
    c.edx = savedEdx;

    const uint16_t* time = systemTime(0x0081d37cu);
    c.esi = 0x0081d37cu;
    c.ecx = time[0];
    c.eax = time[1];
    c.edx = time[3];
    c.edx = (c.edx & 0xffff00ffu) | ((c.eax & 0xffu) << 8);
    gp4::call_regs(0x0052affd, c);
    GP4_GLOBAL(uint16_t, 0x0082459c) = static_cast<uint16_t>(c.eax);

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0052acfa, FUN_0052acfa, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
