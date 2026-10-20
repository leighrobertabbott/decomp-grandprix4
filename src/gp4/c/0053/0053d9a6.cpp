#include <gp4/gp4.h>
#include <gp4/flags.h>

// Dispatcher on the mode word 0x0082fd9c (only when bit 1 of 0x0082fd6a is set); modes 10+ use 9.
// 3/5/8 set the byte 0x0082fd84 to 0xff, 4/6/7 clear it, 0 and 1 do nothing, 9 calls 0x0054e232
// (a bare ret) inside a pushal/popal. Mode 2 looks up the record covering (ebx = [0x0082fd94],
// eax = [0x0082fd98]) with 0x0053dd5f, stores its edx at 0x0082fd88 and, when found, records the
// 0x00539944 / 0x005399ab results in 0x0082fd8c / 0x0082fd90 before setting 0x0082fd84 to 0xff.
// Registers/flags are whatever the callees leave; otherwise edx = 0x0053d9dc + 4 * mode and the
// flags are those of that `add`.

// FUNCTION: GP4 0x0053d9a6
void __cdecl FUN_0053d9a6(gp4::Regs* r) {
    if ((GP4_GLOBAL(uint8_t, 0x0082fd6a) & 2u) == 0) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(GP4_GLOBAL(uint8_t, 0x0082fd6a) & 2u));
        return;
    }
    const uint32_t ebx = GP4_GLOBAL(uint32_t, 0x0082fd94);
    const uint32_t eax = GP4_GLOBAL(uint32_t, 0x0082fd98);
    uint32_t mode = GP4_GLOBAL(uint32_t, 0x0082fd9c);
    if (mode >= 0xau) mode = 9u;
    const uint32_t slot = (mode << 2) + 0x0053d9dcu;
    uint32_t arith = gp4::flags::add32(mode << 2, 0x0053d9dcu);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = eax;
    c.ebx = ebx;
    c.edx = slot;
    c.eflags = gp4::flags::merge(c.eflags, arith);

    switch (mode) {
    case 0:
    case 1:
        break;
    case 2: {
        gp4::call_regs(0x0053dd5f, c);
        GP4_GLOBAL(uint32_t, 0x0082fd88) = c.edx;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.edx));
        if (c.edx != 0) {
            const uint32_t saved_eax = c.eax;
            const uint32_t saved_ebx = c.ebx;
            c.ebx = GP4_FIELD(uint32_t, c.esi, 0xe);
            gp4::call_regs(0x00539944, c);
            GP4_GLOBAL(uint32_t, 0x0082fd8c) = c.eax;
            gp4::call_regs(0x005399ab, c);
            GP4_GLOBAL(uint32_t, 0x0082fd90) = c.eax;
            c.ebx = saved_ebx;
            c.eax = saved_eax;
        }
        GP4_GLOBAL(uint8_t, 0x0082fd84) = 0xffu;
        break;
    }
    case 3:
    case 5:
    case 8:
        GP4_GLOBAL(uint8_t, 0x0082fd84) = 0xffu;
        break;
    case 4:
    case 6:
    case 7:
        GP4_GLOBAL(uint8_t, 0x0082fd84) = 0u;
        break;
    default: {
        gp4::Regs e = c;
        e.esi = 0x0053da79u;
        gp4::call_regs(0x0054e232, e);
        break;
    }
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x0053d9a6, FUN_0053d9a6, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=menu_0084300a+state_0053d9a6")
