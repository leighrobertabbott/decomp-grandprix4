#include <gp4/gp4.h>
#include <gp4/flags.h>

// ebx points at a record whose dwords +4 and +8 are a row and a column. The 16-byte entry
// (row * 6 + column) of the table at 0x007ab02c has two dwords (+4 and +0xc) whose absolute
// difference is compared with 4: below it the counter at 0x00828293 (unaligned dword) is
// reset to 0xba, otherwise it is incremented.
// Leaves ecx = the entry address, eax = |difference| and the flags of the final cmp / add.

// FUNCTION: GP4 0x00534d02
void __cdecl FUN_00534d02(gp4::Regs* r) {
    const uint32_t row = GP4_FIELD(uint32_t, r->ebx, 4);
    const uint32_t column = GP4_FIELD(uint32_t, r->ebx, 8);
    const uint32_t entry = (row * 6u + column) * 0x10u + 0x007ab02cu;

    uint32_t diff = GP4_FIELD(uint32_t, entry, 4) - GP4_FIELD(uint32_t, entry, 0xc);
    if (static_cast<int32_t>(diff) < 0) {
        diff = 0u - diff;
    }
    uint32_t flags = gp4::flags::sub32(diff, 4u);
    if (diff < 4u) {
        GP4_GLOBAL(uint32_t, 0x00828293) = 0xbau;
    } else {
        const uint32_t before = GP4_GLOBAL(uint32_t, 0x00828293);
        GP4_GLOBAL(uint32_t, 0x00828293) = before + 1u;
        flags = gp4::flags::add32(before, 1u);
    }
    r->eax = diff;
    r->ecx = entry;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00534d02, FUN_00534d02, "regs(ebx:ptr[0x40]:i32[0..50]) -> (eax, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
