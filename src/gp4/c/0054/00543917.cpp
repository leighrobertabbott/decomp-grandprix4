#include <gp4/gp4.h>
#include <gp4/flags.h>

// Written as a sum so the pointer-table base keeps the table global 0x00830b00 visible (the original loads esi
// with 0x00830b00 and adds 0xc).
static __declspec(noinline) uint32_t tableAddr(uint32_t base, uint32_t offset) {
    return base + offset;
}

// Finds the first entry of the record-pointer table at 0x00830b0c (count [0x00830bac]) whose record has bit 7
// set in its byte +9. Found: eax = the entry's index and CF = 0 (the other flags are those of `sub count, ecx`).
// Not found (all entries clear, flags of the last `test`): CF = 1 and eax is left holding the last record pointer
// that `lodsd` loaded. An empty table: eax and the flags are untouched except CF = 1. ecx and esi are saved and
// restored.
// FUNCTION: GP4 0x00543917
void __cdecl FUN_00543917(gp4::Regs* r) {
    const uint32_t count = GP4_GLOBAL(uint32_t, 0x00830bac);
    if (count == 0u) {
        r->eflags = r->eflags | gp4::CF;
        return;
    }

    const uint32_t* table = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(tableAddr(0x00830b00u, 0x0cu)));
    uint32_t remaining = count;
    uint32_t record;
    uint32_t arith;
    for (;;) {
        record = *table++;
        const uint8_t selected = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 9) & 0x80u);
        arith = gp4::flags::logic8(selected);
        if (selected != 0u) {
            const uint32_t index = GP4_GLOBAL(uint32_t, 0x00830bac) - remaining;
            r->eax = index;
            r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(GP4_GLOBAL(uint32_t, 0x00830bac), remaining) & ~gp4::CF);
            return;
        }
        remaining -= 1u;
        if (remaining == 0u) {
            break;
        }
    }
    r->eax = record;
    r->eflags = gp4::flags::merge(r->eflags, arith | gp4::CF);
}
GP4_IMPL(0x00543917, FUN_00543917, "regs(eax:u32, cf:u32[0..1], pf:u32[0..1], af:u32[0..1], zf:u32[0..1], sf:u32[0..1], of:u32[0..1]) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=sort_00542224+recs_005438a0")
