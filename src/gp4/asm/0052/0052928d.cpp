#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item value handler (esi = the item, ebx = the cell it edits, edx = the command). Commands 2 and 3 read the
// cell (0x00539944) and, when value - 1 does not borrow and is not below the item's limit (dword +0x26), write it
// back (0x00539a13) and refresh the menu pages (0x00529408, 0x0052a545, 0x005413e9). Command 1 and any other
// command do nothing. The registers and flags are what the last callee on the path leaves.
// FUNCTION: GP4 0x0052928d
void __cdecl FUN_0052928d(gp4::Regs* r) {
    const uint32_t command = r->edx;
    if (command == 1u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(command, 1u));
        return;
    }
    if (command != 2u && command != 3u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(command, 3u));
        return;
    }

    gp4::Regs c = *r;
    gp4::call_regs(0x00539944, c);
    const uint32_t before = c.eax;
    c.eax = before - 1u;
    uint32_t flags = gp4::flags::sub32(before, 1u);
    if (before < 1u) {
        c.eflags = gp4::flags::merge(c.eflags, flags);
        *r = c;
        return;
    }
    const uint32_t limit = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(c.esi + 0x26u));
    flags = gp4::flags::sub32(c.eax, limit);
    c.eflags = gp4::flags::merge(c.eflags, flags);
    if (c.eax < limit) {
        *r = c;
        return;
    }
    gp4::call_regs(0x00539a13, c);
    gp4::call_regs(0x00529408, c);
    gp4::call_regs(0x0052a545, c);
    gp4::call_regs(0x005413e9, c);
    *r = c;
}
GP4_IMPL(0x0052928d, FUN_0052928d, "regs(edx:u32{1,2,3,2,3,0,4}, ebx:ptr[8]:layout=cell_005292c0, esi:ptr[0x40]:layout=item_00529148) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=session_004fd1a0+trk_007ad894+ramp_0081d1f2+menu_00529408")
