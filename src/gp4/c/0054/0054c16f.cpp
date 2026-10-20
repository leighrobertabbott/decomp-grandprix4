#include <gp4/gp4.h>
#include <gp4/flags.h>

// Expands the text scripts 0x74d..0x750 and then 0x62d..0x630 with 0x0054d798, each into the next
// 0x34-byte slot of the string table that starts at 0x007ab5d5 ("Keyboard"). Every call runs inside a
// pushal/popal pair, so only the loop registers change: esi = 0x62d, ecx = 0, edi = the slot after
// the eighth one; the flags are those of the final `dec ecx`.
// FUNCTION: GP4 0x0054c16f
void __cdecl FUN_0054c16f(gp4::Regs* r) {
    uint32_t esi = 0x74du;
    uint32_t edi = 0x007ab5d5u;
    uint32_t ecx = 4u;
    uint32_t arith;

    do {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.esi = esi;
        c.edi = edi;
        c.ecx = ecx;
        gp4::call_regs(0x0054d798, c);
        esi += 1u;
        arith = gp4::flags::add32(edi, 0x34u);
        edi += 0x34u;
        arith = gp4::flags::dec32(ecx, arith);
        ecx -= 1u;
    } while (ecx != 0u);

    esi = 0x62du;
    ecx = 4u;
    do {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.esi = esi;
        c.edi = edi;
        c.ecx = ecx;
        gp4::call_regs(0x0054d798, c);
        arith = gp4::flags::add32(edi, 0x34u);
        edi += 0x34u;
        arith = gp4::flags::dec32(ecx, arith);
        ecx -= 1u;
    } while (ecx != 0u);

    r->esi = esi;
    r->edi = edi;
    r->ecx = ecx;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0054c16f, FUN_0054c16f, "regs(eax:u32, ebx:u32, edx:u32, ebp:u32) -> (ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798+script_0054c16f")
