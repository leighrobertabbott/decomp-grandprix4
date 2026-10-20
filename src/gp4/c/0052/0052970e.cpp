#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu-item handler for an item record at esi whose pointer +0xe leads to a value cell. Inside a pushal /
// popal pair it stores 0x7fff through 0x0054cc96, then builds the item's text at the string buffer
// 0x00823100: script id 0x1aa (0x0054d798), the space terminator (0x0054da9d), and script id
// [[esi + 0xe]] + 0xaaf (0x0054d798). popal restores every register, then the menu-item blit 0x0053c135
// draws the item at esi; the flags are those it leaves.
// FUNCTION: GP4 0x0052970e
void __cdecl FUN_0052970e(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.eax = 0x7fffu;
    gp4::call_regs(0x0054cc96, c);

    c.edi = 0x00823100u;
    const uint32_t item = c.esi;
    c.esi = 0x1aau;
    gp4::call_regs(0x0054d798, c);
    gp4::call_regs(0x0054da9d, c);

    const uint32_t cell = GP4_FIELD(uint32_t, item, 0x0e);
    c.esi = GP4_FIELD(uint32_t, cell, 0) + 0x0aafu;
    gp4::call_regs(0x0054d798, c);

    gp4::Regs f = *r;
    f.eflags = c.eflags;
    gp4::call_regs(0x0053c135, f);
    r->eflags = gp4::flags::merge(r->eflags, f.eflags);
}
GP4_IMPL(0x0052970e, FUN_0052970e, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, ebp:u32, esi:ptr[0x40]:layout=rec_005297ba+item_0053a393) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9+pool_00549db9+script_0054d798+script_all_00835846")
