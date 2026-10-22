#include <gp4/gp4.h>
#include <gp4/flags.h>

// Driver name text at the cursor edi. 0x005484eb maps eax (a six-bit driver code) to a driver index
// (0 when unknown); the text script (index + 0xba6) is expanded at edi through 0x0054d798, then the
// script pointer of the id (index + 0xbb1) is fetched through the table lookup 0x0054d1ad. When that
// second string is not empty a '-' (0x2d) is written and the string is expanded after it (0x0054d7c0).
// The result is NUL-terminated at edi (which stays on the NUL). The registers are threaded through the
// callees; the flags are those of `cmp byte [esi], 0` when the second string is empty, else those of
// the expansion.
// FUNCTION: GP4 0x005489a6
void __cdecl FUN_005489a6(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    gp4::call_regs(0x005484eb, c);
    const uint32_t index = c.eax;

    c.esi = index + 0xba6u;
    gp4::call_regs(0x0054d798, c);

    c.esi = index + 0xbb1u;
    gp4::call_regs(0x0054d1ad, c);

    const uint8_t* second = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(c.esi));
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(second[0], 0u));
    if (second[0] != 0u) {
        *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.edi)) = 0x2du;
        c.edi = c.edi + 1u;
        gp4::call_regs(0x0054d7c0, c);
    }
    *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.edi)) = 0u;

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x005489a6, FUN_005489a6, "regs(eax:u32[0..0x3f], ebx:u32, ecx:u32, edx:u32, edi:ptr[0x100]:bytes, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798+script_all_00835846+drv22_0052d402")
