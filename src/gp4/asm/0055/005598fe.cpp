#include <gp4/gp4.h>
#include <gp4/flags.h>

// Walks the 0x160-byte records from edi up to (excluding) the record [0x00866ef8] and keeps in ebp the
// record whose squared distance to the record in esi (0x00559894, signed compare, strictly smaller wins)
// is smallest; ebp starts as edi (no record visited). Wrapped in push/pop of eax, ebx, ecx, edx, esi, edi, so
// only ebp changes; the flags are those of the final `cmp edi, [0x00866ef8]` (equal).
// FUNCTION: GP4 0x005598fe
void __cdecl FUN_005598fe(gp4::Regs* r) {
    uint32_t record = r->edi;
    int32_t best = 0x7fffffff;
    uint32_t nearest = record;

    while (record != GP4_GLOBAL(uint32_t, 0x00866ef8)) {
        gp4::Regs c = {};
        c.edx = r->edx;
        c.esi = r->esi;
        c.edi = record;
        c.eflags = r->eflags;
        gp4::call_regs(0x00559894, c);
        if (static_cast<int32_t>(c.eax) < best) {
            best = static_cast<int32_t>(c.eax);
            nearest = record;
        }
        record += 0x160u;
    }

    r->ebp = nearest;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(record, record));
}
GP4_IMPL(0x005598fe, FUN_005598fe, "regs(edx:u32, esi:rec[0x8943a0,0x160,0..33], edi:rec[0x8943a0,0x160,0..1]) -> (ebp, cf, pf, af, zf, sf, of); globals=layout=ring_008943a0+geom_008943a0")
