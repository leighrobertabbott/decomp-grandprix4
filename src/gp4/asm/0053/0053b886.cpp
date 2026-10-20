#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0053b886
// Registers a menu item's rectangle for drawing: the item's value (read at the width of its type
// from the cell [esi+0xe], 0x00539944) is stored by 0x00531af4, then the item's rectangle words
// (+4 x, +6 y, +8 and +0xa, as ebx/eax/ebp/ecx) are inserted into the sorted pool list (0x00549db9),
// the stored word is cleared again (0x00531af4 with eax = 0) and the registers are left adjusted:
// ebx = x - 1, eax = y - 1, ebp = +8 + 1, ecx = +0xa + 1 (the empty routines 0x00531b2e and 0x00531b2b
// bracket the end). The flags are those of the final `inc ecx` with the CF of the `and` in 0x00531af4.
void __cdecl FUN_0053b886(gp4::Regs* r) {
    uint8_t* item = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.ebx = GP4_FIELD(uint32_t, item, 0xe);
    gp4::call_regs(0x00539944, c);
    gp4::call_regs(0x00531af4, c);

    c.ebx = GP4_FIELD(uint16_t, item, 4);
    c.eax = GP4_FIELD(uint16_t, item, 6);
    c.ebp = GP4_FIELD(uint16_t, item, 8);
    c.ecx = GP4_FIELD(uint16_t, item, 0xa);
    gp4::call_regs(0x00549db9, c);
    gp4::cdecl_<0x00531b2e, void>();

    gp4::Regs zero = c;
    zero.eax = 0u;
    gp4::call_regs(0x00531af4, zero);

    const uint32_t ecxBefore = c.ecx;
    r->ebx = c.ebx - 1u;
    r->eax = c.eax - 1u;
    r->ebp = c.ebp + 1u;
    r->ecx = ecxBefore + 1u;
    gp4::cdecl_<0x00531b2b, void>();
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::inc32(ecxBefore, zero.eflags));
}
GP4_IMPL(0x0053b886, FUN_0053b886, "regs(esi:u32{=0x843040,0x843066,0x84308c,0x8430b2,0x8430d8,0x8430fe,0x843128,0x843152,0x84317c,0x8431a6,0x8431d0,0x8431fa,0x843224}) -> (eax, ebx, ecx, ebp, cf, pf, af, zf, sf, of); globals=layout=menu_0084300a+list_00549db9")
