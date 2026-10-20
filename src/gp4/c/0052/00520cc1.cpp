#include <gp4/gp4.h>
#include <gp4/flags.h>

// The original indexes the dword table with the constant edx = 3 ([edx*4 + 0x679b9c]); the compiler would
// fold that into an absolute address and hide the table base.
static __declspec(noinline) int32_t tableEntry(const int32_t* base, int32_t index) {
    return base[index];
}

// pushal wrapper: slot = the fourth entry of the table 0x00679b9c ([0x00679ba8]); the slot record is
// [0x007a8e18] + slot * [0x006975c4] (imul, edx:eax the 64-bit product). When its tag dword is 3 the slot
// is loaded back into the game state (0x00520b65, eax = ebp = slot, edx = high half of the product,
// esi = the record) and the one-shot stash is restored (0x00520d28). popal restores every GPR; the flags
// are those of `cmp [esi], 3` or, when the slot was loaded, the last callee's.
// FUNCTION: GP4 0x00520cc1
void __cdecl FUN_00520cc1(gp4::Regs* r) {
    const int32_t slot = tableEntry(GP4_ARRAY(int32_t, 0x00679b9c), 3);
    const int64_t product = static_cast<int64_t>(slot) * static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x006975c4));
    const uint32_t record = static_cast<uint32_t>(product) + GP4_GLOBAL(uint32_t, 0x007a8e18);
    const uint32_t tag = GP4_FIELD(uint32_t, record, 0);
    uint32_t flags = gp4::flags::sub32(tag, 3u);
    if (tag == 3u) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.eax = static_cast<uint32_t>(slot);
        c.ebp = static_cast<uint32_t>(slot);
        c.edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
        c.esi = record;
        c.eflags = gp4::flags::merge(c.eflags, flags);
        gp4::call_regs(0x00520b65, c);
        gp4::call_regs(0x00520d28, c);
        flags = c.eflags;
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00520cc1, FUN_00520cc1, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=rows_00520584+slot_00520b1b+copy_0063b814+links_0066fef0+ptrs_0066d818+ring_008943a0+records_008943a0+segw_00441e25+groups_00868556+w1a_008943a0+segbits_00441e25+words_008943a0+ptr_0086850c+idx_00520b65+rows_00520c5b+once_00520cf4+stash_0085306d")
