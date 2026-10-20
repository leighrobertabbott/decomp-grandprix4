#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit CMP left, right.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Event-list step: finds the record for the clock in the list at 0x00630238 (cursor [0x0063022c], end
// [0x00630230], both byte offsets) with 0x0044214c; when it is found the cursor moves to the next record
// (0xc bytes after a key with bit 31, else the dword +4 further) and 0x00442219 plays that record.
// pushal / popal restore every register; the flags are those of the failed search, or of the final cmp.
// FUNCTION: GP4 0x0044226c
void __cdecl FUN_0044226c(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.edi = GP4_GLOBAL(uint32_t, 0x0063022c) + 0x00630238u;
    c.esi = GP4_GLOBAL(uint32_t, 0x00630230) + 0x00630238u;
    gp4::call_regs(0x0044214c, c);

    uint32_t flags = c.eflags & 0x8d5u;
    if ((c.eflags & 1u) != 0u) {
        const uint32_t record = c.edi;
        const uint32_t* header = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(record));
        const uint32_t next = (header[0] & 0x80000000u) == 0u ? header[1] + record : 0xcu + record;
        GP4_GLOBAL(uint32_t, 0x0063022c) = next - 0x00630238u;
        c.eax = next;
        gp4::call_regs(0x00442219, c);
        flags = cmpFlags32(next, c.edi);
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0044226c, FUN_0044226c, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=clock_0066d814+list_0044214c+state_00433ff8+cursor_0063022c")
