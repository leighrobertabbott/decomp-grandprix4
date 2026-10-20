#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item handler: registers the item's rectangle through 0x543794 (esi = item record; it leaves eax = key,
// ebx, ecx, ebp and dl), then looks up the channel record the cell at [esi+0xe] selects (its dword is an index into the
// 0x20-byte records from 0x00830bb4, or -1 for none). Inside a pushal/popal pair the stdcall routine 0x004fe160 is
// called with (record address or -1, cell); the record address (or -1) is left in edi. The tail 0x543682 draws the
// record's row (it is a pushal/popal wrapper) and leaves the flags.
// FUNCTION: GP4 0x005435f3
void __cdecl FUN_005435f3(gp4::Regs* r) {
    gp4::Regs rect = *r;
    gp4::call_regs(0x00543794, rect);

    const uint32_t cell = GP4_FIELD(uint32_t, r->esi, 0xe);
    uint32_t record = GP4_FIELD(uint32_t, cell, 0);
    if (record != 0xffffffffu) {
        record = record * 0x20u + 0x00830bb4u;
    }

    gp4::Regs lookup = rect;
    const uint32_t args[2] = {record, cell};
    gp4::call_regs_stdcall(0x004fe160, lookup, args);

    gp4::Regs draw = rect;
    draw.edi = record;
    draw.eflags = lookup.eflags;
    gp4::call_regs(0x00543682, draw);

    r->eax = draw.eax;
    r->ebx = draw.ebx;
    r->ecx = draw.ecx;
    r->edx = draw.edx;
    r->ebp = draw.ebp;
    r->edi = draw.edi;
    r->eflags = gp4::flags::merge(r->eflags, draw.eflags);
}
GP4_IMPL(0x005435f3, FUN_005435f3, "regs(ebx:u32, ecx:u32, edx:u32, esi:u32{=0x8460da,0x846100}) -> (eax, ebx, ecx, edx, ebp, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9+chan_008312cc+recs_005438a0+chanrec_00830bb4+script_0054d798+script_0052ad37+script_00543682+textout_0054d211+selrow_005435f3")
