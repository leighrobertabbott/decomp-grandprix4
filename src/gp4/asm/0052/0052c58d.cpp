#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item value handler (esi = the item, ebx = the cell it edits, edx = the command). Command 1 clears edx and does
// nothing else. Commands 2 and 3 read the cell (0x00539944), step it down by one, raise it to the item's lower limit
// (dword +0x26) when it borrowed or is below it, store it back (0x00539a13) and run the page refresh 0x0052bae5. Other
// commands only leave the flags of `cmp edx, 3`. The registers and flags are what the last callee on the path leaves.
// FUNCTION: GP4 0x0052c58d
void __cdecl FUN_0052c58d(gp4::Regs* r) {
    const uint32_t command = r->edx;
    if (command == 1u) {
        r->edx = 0u;
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
    const uint32_t limit = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(c.esi + 0x26u));
    uint32_t flags = gp4::flags::sub32(before, 1u);
    c.eax = before - 1u;
    if (before == 0u) {
        c.eax = limit;
    } else {
        flags = gp4::flags::sub32(c.eax, limit);
        if (c.eax < limit) {
            c.eax = limit;
        }
    }
    c.eflags = gp4::flags::merge(c.eflags, flags);
    gp4::call_regs(0x00539a13, c);
    gp4::call_regs(0x0052bae5, c);
    *r = c;
}
GP4_IMPL(0x0052c58d, FUN_0052c58d, "regs(edx:u32{1,2,3,2,3,0,4}, ebx:ptr[8]:layout=cell_005292c0, esi:ptr[0x40]:layout=item_00529148) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=set_0081d60f+tab_0081dc63+flags_0052bca6+script_0054d798+strs_0052c6be")
