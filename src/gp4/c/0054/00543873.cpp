#include <gp4/gp4.h>
#include <gp4/flags.h>

// Finds the first of the [0x00830bac] datalog channel records (0x20 bytes each, from 0x00830bb4) whose byte +9 has
// bit 7 set. Found: eax = the record's index and CF = 0 (the other flags are those of `sub count, ecx`). Not found
// (flags of the last `add esi, 0x20`) or an empty table (flags untouched): eax = 0 and CF = 1. ecx and esi are
// saved and restored.
// FUNCTION: GP4 0x00543873
void __cdecl FUN_00543873(gp4::Regs* r) {
    const uint32_t count = GP4_GLOBAL(uint32_t, 0x00830bac);
    if (count == 0u) {
        r->eax = 0u;
        r->eflags = r->eflags | gp4::CF;
        return;
    }

    const uint8_t* record = GP4_ARRAY(uint8_t, 0x00830bb4);
    uint32_t remaining = count;
    uint32_t arith;
    for (;;) {
        if ((record[9] & 0x80u) != 0u) {
            r->eax = GP4_GLOBAL(uint32_t, 0x00830bac) - remaining;
            r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(GP4_GLOBAL(uint32_t, 0x00830bac), remaining) & ~gp4::CF);
            return;
        }
        arith = gp4::flags::add32(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(record)), 0x20u);
        record += 0x20;
        remaining -= 1u;
        if (remaining == 0u) {
            break;
        }
    }
    r->eax = 0u;
    r->eflags = gp4::flags::merge(r->eflags, arith | gp4::CF);
}
GP4_IMPL(0x00543873, FUN_00543873, "regs(eax:u32, cf:u32[0..1], pf:u32[0..1], af:u32[0..1], zf:u32[0..1], sf:u32[0..1], of:u32[0..1]) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=recs_005438a0")
