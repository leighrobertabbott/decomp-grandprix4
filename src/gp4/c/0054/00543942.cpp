#include <gp4/gp4.h>
#include <gp4/flags.h>

// The original loads edi with the block address 0x00830b00 and reads the fields at +4 / +8 through it;
// keep the base visible instead of letting the compiler turn the fields into extra globals.
static __declspec(noinline) uint32_t Block() {
    return 0x00830b00u;
}

// Moves the cursor [0x00830b00] of the circular range [.., [0x00830b08]) used by the datalog channel list
// (count [0x00830b04]). When the count does not exceed the limit the cursor is reset to 0. Otherwise
// 0x00543917 looks for the first channel record with bit 7 set in its byte +9 (eax = its index, CF clear;
// CF set: none, nothing else changes): an index below the cursor becomes the cursor, then the offset
// d = index - cursor (cursor <= index) is kept if d < limit, else the cursor wraps to
// index - limit + 1 (written back as cursor + d - limit + 1).
// Exit registers: edi = 0x00830b00, ecx = the count, eax = the last computed value; the flags are those of the
// last flag writer on the path.
// FUNCTION: GP4 0x00543942
void __cdecl FUN_00543942(gp4::Regs* r) {
    const uint32_t block = Block();
    const int32_t count = GP4_FIELD(int32_t, block, 4);
    const uint32_t limit = GP4_FIELD(uint32_t, block, 8);
    uint32_t flags = gp4::flags::sub32(static_cast<uint32_t>(count), limit);
    uint32_t eax = r->eax;

    if (count > static_cast<int32_t>(limit)) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.ecx = static_cast<uint32_t>(count);
        c.edi = block;
        c.eflags = gp4::flags::merge(c.eflags, flags);
        gp4::call_regs(0x00543917, c);
        eax = c.eax;
        flags = c.eflags;
        if ((c.eflags & gp4::CF) == 0u) {
            flags = gp4::flags::sub32(eax, GP4_FIELD(uint32_t, block, 0));
            if (eax < GP4_FIELD(uint32_t, block, 0)) {
                GP4_FIELD(uint32_t, block, 0) = eax;
            }
            flags = gp4::flags::sub32(eax, GP4_FIELD(uint32_t, block, 0));
            eax -= GP4_FIELD(uint32_t, block, 0);
            flags = gp4::flags::sub32(eax, GP4_FIELD(uint32_t, block, 8));
            if (eax >= GP4_FIELD(uint32_t, block, 8)) {
                flags = gp4::flags::add32(eax, GP4_FIELD(uint32_t, block, 0));
                eax += GP4_FIELD(uint32_t, block, 0);
                flags = gp4::flags::sub32(eax, GP4_FIELD(uint32_t, block, 8));
                eax -= GP4_FIELD(uint32_t, block, 8);
                flags = gp4::flags::add32(eax, 1u);
                eax += 1u;
                GP4_FIELD(uint32_t, block, 0) = eax;
            }
        }
    } else {
        GP4_FIELD(uint32_t, block, 0) = 0u;
    }

    r->eax = eax;
    r->ecx = static_cast<uint32_t>(count);
    r->edi = block;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00543942, FUN_00543942, "regs(eax:u32) -> (eax, ecx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=sort_00542224+recs_005438a0+cur_00543942")
