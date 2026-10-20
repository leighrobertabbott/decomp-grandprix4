#include <gp4/gp4.h>

// Address of the SYSTEMTIME buffer; kept out of line so the buffer base is the only global the field reads see
// (the original indexes it through esi).
static __declspec(noinline) const uint16_t* systemTime(uint32_t address) {
    return reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(address));
}

// Reads the local time into the SYSTEMTIME buffer 0x0081d37c (GetLocalTime through the thunk 0x005a493c; ebx, ecx and
// edx are saved around the call) and returns the clock as packed registers: eax = wSecond, ecx = wMinute with
// its high byte replaced by the low byte of wHour, edx = the low byte of wSecond in bits 8..15. esi is saved and
// restored; the flags are what the thunk leaves.
// FUNCTION: GP4 0x0052accd
void __cdecl FUN_0052accd(gp4::Regs* r) {
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
    const uint32_t hour = time[4];
    const uint32_t minute = time[5];
    const uint32_t second = time[6];

    r->eax = second;
    r->ecx = (minute & 0xffff00ffu) | ((hour & 0xffu) << 8);
    r->edx = (second & 0xffu) << 8;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x0052accd, FUN_0052accd, "regs() -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
