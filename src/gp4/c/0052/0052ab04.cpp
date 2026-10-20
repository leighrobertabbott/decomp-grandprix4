#include <gp4/gp4.h>
#include <gp4/flags.h>

// Walks all 0x16 x 0x11 (record, entry) pairs of the 0xf28-byte record array at 0x007adb4c and, for each,
// computes the record address (0x0052a254, eax = record) and then the 0xe4-byte entry address inside it
// (0x0052a260, eax = record, ebx = entry). Both callees only return an address; every call sits between
// pushal/popal, so registers are unchanged. The flags are those of the final `cmp eax, 0x16`.
// FUNCTION: GP4 0x0052ab04
void __cdecl FUN_0052ab04(gp4::Regs* r) {
    uint32_t arith = 0u;
    uint32_t flags = r->eflags;
    uint32_t record = 0u;
    do {
        uint32_t entry = 0u;
        do {
            gp4::Regs first = *r;
            first.eax = record;
            first.ebx = entry;
            first.eflags = flags;
            gp4::call_regs(0x0052a254, first);

            gp4::Regs second = *r;
            second.eax = record;
            second.ebx = entry;
            second.eflags = first.eflags;
            gp4::call_regs(0x0052a260, second);

            ++entry;
            arith = gp4::flags::sub32(entry, 0x11u);
            flags = gp4::flags::merge(second.eflags, arith);
        } while (entry < 0x11u);
        ++record;
        arith = gp4::flags::sub32(record, 0x16u);
        flags = gp4::flags::merge(flags, arith);
    } while (record < 0x16u);
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0052ab04, FUN_0052ab04, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
