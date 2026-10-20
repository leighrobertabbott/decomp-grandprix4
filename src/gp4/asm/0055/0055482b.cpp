#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0055482b
// Closes the track-segment ring at 0x008943a0 (0x160-byte records) around the lap seam:
// record [0x866ecc] (lap end) is overwritten with a copy of record [0x891504] (lap start), and the
// record just before the lap start is overwritten with a copy of the record just before the lap
// end. Two forward `rep movsb` of 0x160 bytes (direction flag clear, as the game runs everywhere).
// Leaves ecx = 0, edi = [0x891504], esi = [0x866ecc] and the flags of the last `sub edi, 0x160`.
void __cdecl FUN_0055482b(gp4::Regs* r) {
    const uint32_t lapEnd = GP4_GLOBAL(uint32_t, 0x00866ecc);
    const uint32_t lapStart = GP4_GLOBAL(uint32_t, 0x00891504);

    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(lapEnd));
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(lapStart));
    for (uint32_t i = 0; i != 0x160u; ++i) {
        dst[i] = src[i];
    }

    const uint32_t beforeStart = lapStart - 0x160u;
    dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(beforeStart));
    src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(lapEnd - 0x160u));
    for (uint32_t i = 0; i != 0x160u; ++i) {
        dst[i] = src[i];
    }

    r->ecx = 0;
    r->esi = lapEnd;
    r->edi = lapStart;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(lapStart, 0x160u));
}
GP4_IMPL(0x0055482b, FUN_0055482b, "regs() -> (ecx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=ring_008943a0")
