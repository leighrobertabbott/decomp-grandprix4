#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item handler keyed by edx. Only edx == 2 acts. esi is the item record; its dword at +0xe points at a cell.
// 0x543873 finds the first datalog channel record (0x20 bytes each from 0x00830bb4) whose selection bit is set; CF set
// means there is none and the handler returns with that routine's eax and flags. Otherwise the record index is stored
// into the cell and, inside a pushal/popal pair, the stdcall routine 0x004fe160 is called with (record address or -1,
// cell). The cell is then compared against the two handle dwords 0x008312cc / 0x008312d0 (the other one when the cell is
// 0x008312cc): when the index equals that dword it is reset to -1 and 0x004fe160 is called again (0, that dword).
// popal restores every register; the flags are those of the last compare or callee.
// FUNCTION: GP4 0x005437f4
void __cdecl FUN_005437f4(gp4::Regs* r) {
    const uint32_t key = r->edx;
    const uint32_t compare = gp4::flags::sub32(key, 2u);
    if (key != 2u) {
        r->eflags = gp4::flags::merge(r->eflags, compare);
        return;
    }

    const uint32_t cell = GP4_FIELD(uint32_t, r->esi, 0xe);
    r->edi = cell;

    gp4::Regs found = *r;
    found.eflags = gp4::flags::merge(r->eflags, compare);
    gp4::call_regs(0x00543873, found);
    r->eax = found.eax;
    if ((found.eflags & gp4::CF) != 0u) {
        r->eflags = gp4::flags::merge(r->eflags, found.eflags);
        return;
    }

    const uint32_t index = found.eax;
    GP4_FIELD(uint32_t, cell, 0) = index;
    uint32_t record = GP4_FIELD(uint32_t, cell, 0);
    if (record != 0xffffffffu) {
        record = record * 0x20u + 0x00830bb4u;
    }

    gp4::Regs first = *r;
    first.eflags = found.eflags;
    const uint32_t firstArgs[2] = {record, cell};
    gp4::call_regs_stdcall(0x004fe160, first, firstArgs);

    uint32_t handle = 0x008312ccu;
    uint32_t flags = gp4::flags::sub32(handle, cell);
    if (handle == cell) {
        handle = 0x008312d0u;
    }
    r->ebx = handle;
    const uint32_t current = GP4_FIELD(uint32_t, handle, 0);
    flags = gp4::flags::sub32(index, current);
    if (index != current) {
        r->eflags = gp4::flags::merge(r->eflags, flags);
        return;
    }

    GP4_FIELD(uint32_t, handle, 0) = 0xffffffffu;
    gp4::Regs second = *r;
    second.eflags = gp4::flags::merge(r->eflags, flags);
    const uint32_t secondArgs[2] = {0u, handle};
    gp4::call_regs_stdcall(0x004fe160, second, secondArgs);
    r->eflags = gp4::flags::merge(r->eflags, second.eflags);
}
GP4_IMPL(0x005437f4, FUN_005437f4, "regs(edx:u32{=2,2,2,0,1,3}, esi:u32{=0x845e1c,0x845e42}) -> (eax, ebx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cur_00543942+recs_005438a0+chan_008312cc+chanrec_00830bb4+script_0054d798+script_0052ad37+script_00543682")
