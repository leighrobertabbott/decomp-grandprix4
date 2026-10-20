#include <gp4/gp4.h>
#include <gp4/flags.h>

// The two 0x0054cb71 calls (an empty save/restore routine) get one helper each so both stay visible.
static __declspec(noinline) void run_reset_a(gp4::Regs* c) {
    gp4::call_regs(0x0054cb71, *c);
}

static __declspec(noinline) void run_reset_b(gp4::Regs* c) {
    gp4::call_regs(0x0054cb71, *c);
}

// Draws the two-line caption of a menu item whose record is esi: sets the text colour (0x0054cc96 with
// eax = 0x7fff), expands the script of text id 0x13f into the string buffer 0x00823100 and then the script of
// id 0x140 (0x141 when the bytes [0x0081d60a] and [0x0081d60b] differ) at the position the first expansion
// left in edi, and finally blits the item with 0x0053c135 (esi = the item). Registers are what the
// second expansion leaves, except esi (restored) and eax = 1; the flags are those of 0x0053c135.
// FUNCTION: GP4 0x0053b7d0
void __cdecl FUN_0053b7d0(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.eax = 0x7fffu;
    gp4::call_regs(0x0054cc96, c);
    c.eax = 0u;
    run_reset_a(&c);

    const uint32_t item = r->esi;
    c.edi = 0x00823100u;
    c.esi = 0x13fu;
    gp4::call_regs(0x0054d798, c);

    c.esi = 0x140u;
    const uint32_t first = GP4_GLOBAL(uint8_t, 0x0081d60a);
    c.eax = (c.eax & 0xffffff00u) | first;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(first, GP4_GLOBAL(uint8_t, 0x0081d60b)));
    if (first != GP4_GLOBAL(uint8_t, 0x0081d60b)) {
        c.esi = 0x141u;
    }
    gp4::call_regs(0x0054d798, c);

    c.esi = item;
    gp4::call_regs(0x0053c135, c);
    c.eax = 1u;
    run_reset_b(&c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0053b7d0, FUN_0053b7d0, "regs(ebx:u32, ecx:u32, edx:u32, esi:ptr[0x10]:bytes, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798+list_00549db9+pool_00549db9+textout_0054d211+script_0053b7d0")
