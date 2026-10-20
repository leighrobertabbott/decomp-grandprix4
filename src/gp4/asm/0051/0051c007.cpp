#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0051c007
// pushal wrapper over a table of [0x0096a368] 0x20-byte records at esi. [0x00965908] is set to the
// table end (esi + count * 0x20); [0x0096590c] is that end plus 4 * (word at +0x10) for every record
// whose word +0x14 has neither bit 0x80 nor bit 0x04 set and whose word +0x1a has no bit 0x04.
// popal restores every GPR; the flags are those of the final `dec ebp` (1 -> 0) with CF kept from the
// last `add ebx, 0x20`.
void __cdecl FUN_0051c007(gp4::Regs* r) {
    const uint32_t first = r->esi;
    const uint32_t tableEnd = GP4_GLOBAL(uint32_t, 0x0096a368) * 0x20u + first;
    GP4_GLOBAL(uint32_t, 0x00965908) = tableEnd;

    uint32_t cursor = tableEnd;
    uint32_t record = first;
    uint32_t remaining = GP4_GLOBAL(uint32_t, 0x0096a368);
    uint32_t advanceFlags = 0u;
    uint32_t before;
    do {
        uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(record));
        const uint16_t flags14 = GP4_FIELD(uint16_t, rec, 0x14);
        if ((flags14 & 0x80u) == 0u && (flags14 & 0x04u) == 0u && (GP4_FIELD(uint16_t, rec, 0x1a) & 0x04u) == 0u) {
            cursor = cursor + static_cast<uint32_t>(GP4_FIELD(uint16_t, rec, 0x10)) * 4u;
        }
        advanceFlags = gp4::flags::add32(record, 0x20u);
        record = record + 0x20u;
        before = remaining;
        --remaining;
    } while (remaining != 0u);
    GP4_GLOBAL(uint32_t, 0x0096590c) = cursor;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(before, advanceFlags));
}
GP4_IMPL(0x0051c007, FUN_0051c007, "regs(esi:ptr[0x220]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=count_0096a368")
