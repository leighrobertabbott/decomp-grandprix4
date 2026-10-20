#include <gp4/gp4.h>
#include <gp4/flags.h>

// Walks the ring of 0x160-byte records from esi until it reaches the record edx (wrapping to 0x008943a0 when
// the cursor reaches [0x00866ecc]) and returns in eax the record whose squared distance to the record in
// edi (0x00559894, signed compare, strictly smaller wins) is smallest; with esi == edx nothing is visited and
// eax = esi. Wrapped in push/pop of ebx, ecx, edx, ebp, esi, edi, so only eax changes; the flags are those of
// the final `cmp esi, edx` (equal).
// FUNCTION: GP4 0x005598c0
void __cdecl FUN_005598c0(gp4::Regs* r) {
    uint32_t cursor = r->esi;
    const uint32_t stop = r->edx;
    int32_t best = 0x7fffffff;
    uint32_t nearest = cursor;

    while (cursor != stop) {
        gp4::Regs c = {};
        c.edx = stop;
        c.esi = cursor;
        c.edi = r->edi;
        c.eflags = r->eflags;
        gp4::call_regs(0x00559894, c);
        if (static_cast<int32_t>(c.eax) < best) {
            best = static_cast<int32_t>(c.eax);
            nearest = cursor;
        }
        cursor += 0x160u;
        if (!(cursor < GP4_GLOBAL(uint32_t, 0x00866ecc))) {
            cursor = 0x008943a0u;
        }
    }

    r->eax = nearest;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(cursor, cursor));
}
GP4_IMPL(0x005598c0, FUN_005598c0, "regs(edx:rec[0x8943a0,0x160,0..3], esi:rec[0x8943a0,0x160,0..3], edi:rec[0x8943a0,0x160,0..33]) -> (eax, cf, pf, af, zf, sf, of); globals=layout=ring_008943a0+geom_008943a0")
