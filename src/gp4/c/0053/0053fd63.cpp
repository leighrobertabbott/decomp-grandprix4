#include <gp4/gp4.h>
#include <gp4/flags.h>

// Counts the stale entries of the 22 records at 0xb1b8d8 (stride 0x1c): the performance counter
// (0x0054e733, edx:eax) is stored in the qword at 0x830004 and every record whose id (first dword) is
// non-zero and differs from the current id [0xb1bb40] is compared with its qword time stamp (0x83002b,
// 8 bytes apiece): the entry counts when the 64-bit difference now - stamp is greater than 0x4446390
// (signed high half above zero, or zero with the low half above the limit, unsigned). pushes/pops
// ebx, ecx, edx, esi keep them; eax and edi end as the count, and the flags are those of the closing
// `or ecx, ecx` with ecx = 0.
// FUNCTION: GP4 0x0053fd63
void __cdecl FUN_0053fd63(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0054e733, c);
    GP4_GLOBAL(uint32_t, 0x00830004) = c.eax;
    GP4_GLOBAL(uint32_t, 0x00830008) = c.edx;

    uint32_t record = 0x00b1b8d8u;
    uint32_t stamp = 0x0083002bu;
    uint32_t count = 0u;
    for (uint32_t left = 0x16u; left != 0u; --left) {
        const uint32_t id = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(record));
        if (id != 0u && id != GP4_GLOBAL(uint32_t, 0x00b1bb40)) {
            const uint32_t nowLow = GP4_GLOBAL(uint32_t, 0x00830004);
            const uint32_t nowHigh = GP4_GLOBAL(uint32_t, 0x00830008);
            const uint32_t stampLow = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(stamp));
            const uint32_t stampHigh = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(stamp + 4u));
            const uint32_t low = nowLow - stampLow;
            const uint32_t high = nowHigh - stampHigh - (nowLow < stampLow ? 1u : 0u);
            if (static_cast<int32_t>(high) > 0 || (high == 0u && low > 0x4446390u)) {
                ++count;
            }
        }
        record += 0x1cu;
        stamp += 8u;
    }

    r->eax = count;
    r->edi = count;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(0u));
}
GP4_IMPL(0x0053fd63, FUN_0053fd63, "regs() -> (eax, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=stamps_0053fd63")
