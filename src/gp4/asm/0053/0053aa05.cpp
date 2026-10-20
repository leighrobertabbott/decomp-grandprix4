#include <gp4/gp4.h>
#include <gp4/flags.h>

// Draw handler of a menu value-text item (esi = the item record). Unless bit 4 of the item flag byte +0x1f is
// set: 0x54cc96 sets the draw colour (0x5298 when bit 5 of +0x1f is set, else 0x7fff); 0x53ab3a queues the item
// rectangle (words +4/+6/+8/+0xa) with the style offset 0x72 when the byte +0x32 is nonzero, else 0x77; the
// inner box (left + 0x14, top - 3 (+5 when +0x32 is zero), right word +0x2e, bottom +0xa (+5 likewise)) is
// queued through 0x549db9; 0x54cd42 latches the text rectangle (left + 0x1a, bottom + 0x17 - ..., width
// +0x2e - 4) and 0x54cb7e stores its left edge; the item value is read through 0x539944 (cell [esi+0xe], width
// by +0xd) and the text-table entry value + [esi+0x12] is drawn by 0x54d1a2 before 0x54cd61 clears the latch.
// The colour is then reset to 0x7fff through 0x54cc96 (also on the early exit). pushal/popal restore every
// register; the flags are those of that last 0x54cc96 call.
// FUNCTION: GP4 0x0053aa05
void __cdecl FUN_0053aa05(gp4::Regs* r) {
    const uint8_t* item = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t itemFlags = item[0x1f];
    gp4::Regs c = *r;

    if ((itemFlags & 0x10u) == 0u) {
        c.eax = 0x7fffu;
        if ((itemFlags & 0x20u) != 0u) {
            c.eax = 0x5298u;
        }
        gp4::call_regs(0x0054cc96, c);

        const bool plain = item[0x32] == 0u;
        c.ebx = *reinterpret_cast<const uint16_t*>(item + 0x04);
        c.eax = *reinterpret_cast<const uint16_t*>(item + 0x06);
        c.edx = plain ? 0x77u : 0x72u;
        gp4::call_regs(0x0053ab3a, c);

        c.ebx = *reinterpret_cast<const uint16_t*>(item + 0x04);
        c.eax = *reinterpret_cast<const uint16_t*>(item + 0x06);
        c.ebp = *reinterpret_cast<const uint16_t*>(item + 0x2e);
        c.ecx = *reinterpret_cast<const uint16_t*>(item + 0x0a);
        if (plain) {
            c.eax += 5u;
            c.ecx += 5u;
        }
        c.ebx += 0x14u;
        c.eax -= 3u;
        gp4::cdecl_<0x0054a1d7, void>();
        gp4::call_regs(0x00549db9, c);

        c.edx = 0x66u;
        gp4::cdecl_<0x00531b37, void>();
        c.ebx += 6u;
        c.ecx = c.eax + 0x17u;
        c.ebp -= 4u;
        gp4::call_regs(0x0054cd42, c);
        gp4::cdecl_<0x0054d70a, void>();
        gp4::call_regs(0x0054cb7e, c);

        c.ebx = *reinterpret_cast<const uint32_t*>(item + 0x0e);
        gp4::call_regs(0x00539944, c);
        c.esi = c.eax + *reinterpret_cast<const uint32_t*>(item + 0x12);
        gp4::call_regs(0x0054d1a2, c);
        gp4::cdecl_<0x0054cd61, void>();
        c.esi = r->esi;
    }

    c.eax = 0x7fffu;
    gp4::call_regs(0x0054cc96, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0053aa05, FUN_0053aa05, "regs(ebx:u32, ecx:u32, edx:u32, esi:ptr[0x40]:layout=rec_005297ba+item_0053a393) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9+pool_00549db9+script_0054d798+textout_0054d211+strs_0053aa05")
