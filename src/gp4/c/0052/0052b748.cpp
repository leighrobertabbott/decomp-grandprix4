#include <gp4/gp4.h>
#include <gp4/flags.h>

// Scans the four 0x34-byte slot records at 0x007ab684 (slots 4..7) for the first whose selector byte
// is 4 and returns its slot number in eax, or -1 when none matches. ecx/esi/edi are saved and
// restored. The flags are those of the last flag writer: "cmp byte [esi], 4" on a match, the final
// "cmp eax, 8" when the scan runs out.
// FUNCTION: GP4 0x0052b748
void __cdecl FUN_0052b748(gp4::Regs* r) {
    const uint8_t* record = GP4_ARRAY(uint8_t, 0x007ab684);
    for (uint32_t slot = 4u; slot != 8u; ++slot) {
        if (record[0] == 4u) {
            r->eax = slot;
            r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(record[0], 4u));
            return;
        }
        record += 0x34;
    }
    r->eax = 0xffffffffu;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(8u, 8u));
}
GP4_IMPL(0x0052b748, FUN_0052b748, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=flags_0052c55a")
