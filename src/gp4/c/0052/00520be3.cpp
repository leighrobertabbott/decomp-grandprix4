#include <gp4/gp4.h>
#include <gp4/flags.h>

// The tables are indexed by the variable edx = [0x679b44]; the compiler would fold the bases into
// absolute addresses and hide them.
static __declspec(noinline) int32_t tableEntry(const int32_t* base, int32_t index) {
    return base[index];
}

// FUNCTION: GP4 0x00520be3
// pushal wrapper of the state loader 0x00520b65: the slot index is the ring position
// [0x679b34 + 4*idx], or the lower bound [0x679b9c + 4*idx] when it is above (signed) the upper limit
// [0x679b24 + 4*idx], with idx = [0x679b44]. popal restores every GPR; the flags are those the loader leaves.
void __cdecl FUN_00520be3(gp4::Regs* r) {
    const int32_t index = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00679b44));
    int32_t slot = tableEntry(GP4_ARRAY(int32_t, 0x00679b34), index);
    const int32_t limit = tableEntry(GP4_ARRAY(int32_t, 0x00679b24), index);
    const uint32_t arith = gp4::flags::sub32(static_cast<uint32_t>(slot), static_cast<uint32_t>(limit));
    if (slot > limit) {
        slot = tableEntry(GP4_ARRAY(int32_t, 0x00679b9c), index);
    }

    gp4::Regs c = *r;
    c.eax = static_cast<uint32_t>(slot);
    c.edx = static_cast<uint32_t>(index);
    c.eflags = gp4::flags::merge(r->eflags, arith);
    gp4::call_regs(0x00520b65, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00520be3, FUN_00520be3, "regs(edi:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slot_00520b1b+copy_0063b814+links_0066fef0+ptrs_0066d818+ring_008943a0+records_008943a0+segw_00441e25+words_008943a0+ptr_0086850c+idx_00520b65+idx_00520be3")
