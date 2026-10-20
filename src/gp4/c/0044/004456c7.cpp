#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `add left, right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x01u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Ranks the 22 table slots and binds them to the car records. Rank pass: for each group g in 0..21 the first
// slot whose key byte (0x0062f916) is not 0xff and whose group byte (0x007c2a32) equals g gets the next rank,
// stored in the byte table 0x0062f900 (the number of ranks goes to the dword [0x0062f8e4]). Car pass: every car
// record (0x0066fef0, stride 0x41c) whose key byte +0x20c is not 0xff and is found in the key table (0x00445a0a)
// receives the slot's rank in +0x20d; if that slot's key equals the byte [0x007ab022] the rank is also kept in
// [0x0062f8ef] and a rank of zero sets [0x007a8df8] to 0xff. Both bytes start as 0 / 0xff. pushal / popal restore
// every register; the flags are those of the last `add esi, 0x41c`.
// FUNCTION: GP4 0x004456c7
void __cdecl FUN_004456c7(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x007a8df8) = 0;
    GP4_GLOBAL(uint8_t, 0x0062f8ef) = 0xff;

    uint8_t* const rank = GP4_ARRAY(uint8_t, 0x0062f900);
    const uint8_t* const key = GP4_ARRAY(const uint8_t, 0x0062f916);
    const uint8_t* const group = GP4_ARRAY(const uint8_t, 0x007c2a32);

    uint32_t ranked = 0;
    for (uint32_t wanted = 0; wanted != 0x16u; ++wanted) {
        for (uint32_t slot = 0; slot != 0x16u; ++slot) {
            if (key[slot] != 0xffu && static_cast<uint8_t>(wanted) == group[slot]) {
                rank[slot] = static_cast<uint8_t>(ranked);
                ranked += 1u;
                break;
            }
        }
    }
    GP4_GLOBAL(uint32_t, 0x0062f8e4) = ranked;

    uint8_t* car = GP4_ARRAY(uint8_t, 0x0066fef0);
    uint32_t flags = 0;
    for (uint32_t remaining = 0x16; remaining != 0u; --remaining) {
        const uint8_t carKey = car[0x20c];
        if (carKey != 0xffu) {
            gp4::Regs lookup = *r;
            lookup.st_in = 0;
            lookup.st_out = 0;
            lookup.eax = (lookup.eax & 0xffffff00u) | carKey;
            lookup.ecx = remaining;
            lookup.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car));
            gp4::call_regs(0x00445a0a, lookup);
            if ((lookup.eflags & 1u) == 0u) {
                const uint32_t index = lookup.eax;
                const uint8_t slotRank = rank[index];
                car[0x20d] = slotRank;
                const uint8_t slotKey = key[index];
                if (slotKey == GP4_GLOBAL(uint8_t, 0x007ab022)) {
                    GP4_GLOBAL(uint8_t, 0x0062f8ef) = slotRank;
                    if (slotRank == 0u) {
                        GP4_GLOBAL(uint8_t, 0x007a8df8) = 0xff;
                    }
                }
            }
        }
        flags = addFlags32(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car)), 0x41cu);
        car += 0x41c;
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004456c7, FUN_004456c7, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
