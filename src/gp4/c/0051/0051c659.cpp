#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal loop over the object records (0x20 bytes each, array pointer [0x009658fc], count
// [0x0096a368], at least one pass): the object id word at +0x12 is stored in the word [0x009679a4]
// and, when the id's slot word in the table 0x009e1b20 is not 0xffff, the texture entry routine
// 0x0051b5a5 runs for it. popal restores every register, so only the flags remain: those of the
// closing `cmp ebp, [0x96a368]` (equal operands when the loop leaves).
// FUNCTION: GP4 0x0051c659
void __cdecl FUN_0051c659(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    uint32_t record = GP4_GLOBAL(uint32_t, 0x009658fc);
    uint32_t index = 0u;
    do {
        const uint32_t id = GP4_FIELD(uint16_t, record, 0x12);
        GP4_GLOBAL(uint16_t, 0x009679a4) = static_cast<uint16_t>(id);
        const uint32_t slot = GP4_ARRAY(uint16_t, 0x009e1b20)[id];
        c.esi = record;
        c.ebp = index;
        c.eax = slot;
        if (slot != 0xffffu) {
            gp4::call_regs(0x0051b5a5, c);
        }
        record += 0x20u;
        ++index;
    } while (index != GP4_GLOBAL(uint32_t, 0x0096a368));

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(index, GP4_GLOBAL(uint32_t, 0x0096a368)));
}
GP4_IMPL(0x0051c659, FUN_0051c659, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=recs_0051c5fd+entry_0051b5a5+slots_0051cc50")
