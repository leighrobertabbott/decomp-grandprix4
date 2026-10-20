#include <gp4/gp4.h>
#include <gp4/flags.h>

// Latches up to four dwords through optional source pointers: for each pair
// (pointer 0x006f2c0c/10/14/18, destination 0x007ab158/5c/60/64) the destination receives the
// dword the pointer addresses when the pointer is not null. eax ends as the last pointer's target
// value (or 0 when that pointer is null) and the flags are those of the final "or eax,eax"
// (the movs leave them alone).
// FUNCTION: GP4 0x0051dcc7
void __cdecl FUN_0051dcc7(gp4::Regs* r) {
    uint32_t eax = GP4_GLOBAL(uint32_t, 0x006f2c0c);
    uint32_t arith = gp4::flags::logic32(eax);
    if (eax != 0u) {
        eax = *(const uint32_t*)(uintptr_t)eax;
        GP4_GLOBAL(uint32_t, 0x007ab158) = eax;
    }
    eax = GP4_GLOBAL(uint32_t, 0x006f2c10);
    arith = gp4::flags::logic32(eax);
    if (eax != 0u) {
        eax = *(const uint32_t*)(uintptr_t)eax;
        GP4_GLOBAL(uint32_t, 0x007ab15c) = eax;
    }
    eax = GP4_GLOBAL(uint32_t, 0x006f2c14);
    arith = gp4::flags::logic32(eax);
    if (eax != 0u) {
        eax = *(const uint32_t*)(uintptr_t)eax;
        GP4_GLOBAL(uint32_t, 0x007ab160) = eax;
    }
    eax = GP4_GLOBAL(uint32_t, 0x006f2c18);
    arith = gp4::flags::logic32(eax);
    if (eax != 0u) {
        eax = *(const uint32_t*)(uintptr_t)eax;
        GP4_GLOBAL(uint32_t, 0x007ab164) = eax;
    }
    r->eax = eax;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0051dcc7, FUN_0051dcc7, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=srcs_006f2c0c")
