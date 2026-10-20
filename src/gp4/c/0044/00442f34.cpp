#include <gp4/gp4.h>

// Counts the active drivers: walks the per-slot flag bytes at 0x0066d8f4 (one per car, [0x0066d8ec] of them);
// a slot with bit 7 set and bit 6 clear whose car record (0x0066fef0 + slot * 0x41c, the offset wrapped to a
// signed 16-bit value like `movsx esi, ax`) does not have bit 5 of byte +0x7a is counted into the word
// [0x0066d89a], and its byte +0x120 is summed into the word [0x0066d894]. All registers are saved/restored
// (pushal/popal); the loop ends on `cmp ecx, [0x0066d8ec]` (or the initial `test` of a zero count), both
// leaving ZF and PF set.
// FUNCTION: GP4 0x00442f34
void __cdecl FUN_00442f34(gp4::Regs* r) {
    uint32_t count = 0;
    uint32_t sum = 0;
    const uint32_t slots = GP4_GLOBAL(uint32_t, 0x0066d8ec);

    if (slots != 0u) {
        const uint8_t* flagBytes = GP4_ARRAY(uint8_t, 0x0066d8f4);
        uint32_t slot = 0;
        do {
            const uint8_t flagByte = *flagBytes;
            if ((flagByte & 0x80u) != 0u && (flagByte & 0x40u) == 0u) {
                const int16_t offset = static_cast<int16_t>(slot * 0x41cu);
                const uint8_t* car = GP4_ARRAY(uint8_t, 0x0066fef0) + offset;
                if ((car[0x7a] & 0x20u) == 0u) {
                    count += 1u;
                    sum += car[0x120];
                }
            }
            ++flagBytes;
            ++slot;
        } while (slot != GP4_GLOBAL(uint32_t, 0x0066d8ec));
    }

    GP4_GLOBAL(uint16_t, 0x0066d89a) = static_cast<uint16_t>(count);
    GP4_GLOBAL(uint16_t, 0x0066d894) = static_cast<uint16_t>(sum);
    // The last flag writer is `test [0x66d8ec], -1` (zero) or the final `cmp ecx, [0x66d8ec]` (equal): ZF and PF.
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00442f34, FUN_00442f34, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=order_00675958+cars_0066fef0")
