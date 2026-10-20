#include <gp4/gp4.h>

// 0x160-byte track-segment records at 0x008943a0 (only the flag byte this routine tests).
struct Unknown_008943a0 {
    uint8_t Unknown00[0x73];
    uint8_t Flags73;
    uint8_t Unknown74[0x160 - 0x74];
};
static_assert(sizeof(Unknown_008943a0) == 0x160, "record stride");

// Walks the records from 0x008943a0 up to [0x00866ecc] - 0x2c0 and returns the arithmetic flags of
// the last flag writer (pushal / popal leave every GPR alone).
static uint32_t scanRecords(const gp4::Regs* r) {
    const uint32_t end = GP4_GLOBAL(uint32_t, 0x00866ecc) - 0x2c0u;
    uint32_t record = 0x008943a0u;
    for (;;) {
        const uint8_t flags = reinterpret_cast<const Unknown_008943a0*>(record)->Flags73;
        if ((flags & 1u) != 0u && ((flags & 4u) == 0u || (flags & 8u) == 0u)) {
            GP4_GLOBAL(uint8_t, 0x00627b65) = 0u;
            if ((flags & 2u) != 0u) {
                GP4_GLOBAL(uint8_t, 0x00627b65) = 0xffu;
            }
            int32_t sum = 0;
            int32_t count = 0;
            while ((reinterpret_cast<const Unknown_008943a0*>(record)->Flags73 & 0x80u) == 0u) {
                gp4::Regs delta = *r;
                delta.edi = record;
                delta.st_in = 0;
                delta.st_out = 0;
                gp4::call_regs(0x0043fab0, delta);
                sum += (int32_t)delta.eax;
                ++count;
                record += 0x160u;
                if (record == end) {
                    return 0x44u;   // cmp edi, esi (equal): ZF | PF
                }
            }
            if (count == 0) {
                count = 1;
            }
            const int32_t average = sum / count;
            const int32_t remainder = sum % count;
            int32_t magnitude = average;
            if (magnitude < 0) {
                magnitude = (int32_t)(0u - (uint32_t)magnitude);
            }
            if (magnitude >= GP4_GLOBAL(int32_t, 0x00627ac4)) {
                gp4::Regs walk = *r;
                walk.eax = (uint32_t)magnitude;
                walk.ecx = (uint32_t)count;
                walk.edx = (uint32_t)remainder;
                walk.ebp = (uint32_t)average;
                walk.esi = end;
                walk.edi = record;
                walk.st_in = 0;
                walk.st_out = 0;
                gp4::call_regs(0x0043faf5, walk);
                if ((walk.eflags & 1u) != 0u) {
                    return walk.eflags & 0x8d5u;   // jae not taken: leave with the walk's flags
                }
            }
        }
        record += 0x160u;
        if (record == end) {
            return 0x44u;   // cmp edi, esi (equal): ZF | PF
        }
    }
}

// FUNCTION: GP4 0x0043fb76
// pushal / popal wrapper. For each record whose flag byte +0x73 has bit 0 set (and not both bits 2
// and 3), it sums the 16-bit deltas of 0x0043fab0 over the records up to the next one with bit 7
// set, averages them and, when the average magnitude reaches [0x00627ac4], lets 0x0043faf5 mark
// the walk; the scan stops as soon as that routine reports CF = 1. Only the flags and memory change.
void __cdecl FUN_0043fb76(gp4::Regs* r) {
    r->eflags = (r->eflags & ~0x8d5u) | scanRecords(r);
}
GP4_IMPL(0x0043fb76, FUN_0043fb76, "regs() -> (cf, pf, af, zf, sf, of); cases=1500; globals=layout=ring_0043faf5+ring_008943a0")
