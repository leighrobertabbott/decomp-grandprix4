#include <gp4/gp4.h>
#include <gp4/flags.h>

// Reads two dwords a, b from esi (lodsd twice) and runs 0x00510fad with ecx = eax = b - a, edx = 0, edi = 0.
// The flags it passes in are those of `sub eax, edx` (b - a); eax, the other registers and the flags that come
// back are the callee's.
// FUNCTION: GP4 0x005138e9
void __cdecl FUN_005138e9(gp4::Regs* r) {
    const uint32_t* src = reinterpret_cast<const uint32_t*>(r->esi);
    const uint32_t first = src[0];
    const uint32_t second = src[1];
    const uint32_t length = second - first;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.esi = r->esi + 8u;
    c.eax = length;
    c.ecx = length;
    c.edx = 0u;
    c.edi = 0u;
    c.eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(second, first));
    gp4::call_regs(0x00510fad, c);

    r->eax = c.eax;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x005138e9, FUN_005138e9, "regs(esi:ptr[0x10]:bytes) -> (eax, ecx, edx, esi, edi, cf, pf, af, zf, sf, of)")
