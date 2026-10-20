#include <gp4/gp4.h>
#include <gp4/flags.h>

// Clears bit 7 of byte +9 in each of the forty 0x20-byte records starting at 0x00830bb4. ecx and edi are
// saved and restored; `loop` writes no flag, so the flags are those of the last `add edi, 0x20`.
// FUNCTION: GP4 0x005438a0
void __cdecl FUN_005438a0(gp4::Regs* r) {
    uint8_t* record = GP4_ARRAY(uint8_t, 0x00830bb4);
    uint32_t flags = 0u;
    for (uint32_t remaining = 0x28u; remaining != 0u; --remaining) {
        record[9] = static_cast<uint8_t>(record[9] & 0x7fu);
        flags = gp4::flags::add32(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(record)), 0x20u);
        record += 0x20;
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005438a0, FUN_005438a0, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=recs_005438a0")
