#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sets the value word 0x7fff, then 0x63ff, through 0x00531af4 (which stores the word in [0x00827559]); between the
// calls it loads the origin dwords [0x0081d1c8] / [0x0081d1cc] into ebx / eax with ebp = ebx + width and ecx = eax +
// 0x8c for the (empty) routine 0x00531b2e. The first rectangle spans x + 0 .. x + 0x100, the second x + 0x108 ..
// x + 0x108 + 0x66. It leaves eax = [0x0081d1cc], ebx = [0x0081d1c8] + 0x108, ebp = ebx + 0x66,
// ecx = eax + 0x8c, and the flags of the final `add ecx, 0x8c`.
// FUNCTION: GP4 0x0052a735
void __cdecl FUN_0052a735(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;

    call.eax = 0x7fffu;
    gp4::call_regs(0x00531af4, call);

    const uint32_t x1 = GP4_GLOBAL(uint32_t, 0x0081d1c8);
    const uint32_t y1 = GP4_GLOBAL(uint32_t, 0x0081d1cc);
    call.ebx = x1;
    call.eax = y1;
    call.ebp = x1 + 0x100u;
    call.ecx = y1 + 0x8cu;
    call.eflags = gp4::flags::merge(call.eflags, gp4::flags::add32(y1, 0x8cu));
    gp4::call_regs(0x00531b2e, call);

    call.eax = 0x63ffu;
    gp4::call_regs(0x00531af4, call);

    const uint32_t x2 = GP4_GLOBAL(uint32_t, 0x0081d1c8);
    const uint32_t y2 = GP4_GLOBAL(uint32_t, 0x0081d1cc);
    call.ebx = x2 + 0x108u;
    call.eax = y2;
    call.ebp = call.ebx + 0x66u;
    call.ecx = y2 + 0x8cu;
    call.eflags = gp4::flags::merge(call.eflags, gp4::flags::add32(y2, 0x8cu));
    gp4::call_regs(0x00531b2e, call);

    r->eax = call.eax;
    r->ebx = call.ebx;
    r->ecx = call.ecx;
    r->ebp = call.ebp;
    r->eflags = gp4::flags::merge(r->eflags, call.eflags);
}
GP4_IMPL(0x0052a735, FUN_0052a735, "regs() -> (eax, ebx, ecx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
