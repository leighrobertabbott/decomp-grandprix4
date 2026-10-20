#include <gp4/gp4.h>
#include <gp4/flags.h>

// Closes a gap in a text buffer (the counterpart of 0x0054e12a): edi is a record whose dword +4 is
// the buffer end pointer E. The end moves down by eax and, when E - edx + 1 - eax > 0, the bytes
// from edx + eax are copied forward (rep movsb) down to edx; otherwise (nothing left to move) the
// no-op routine 0x0054e232 runs. Every register is restored by popal; the flags are those of the
// `add esi, eax` (esi = edx) on the copying path, else of the `sub ecx, eax` that decides the
// branch (ecx = E - edx + 1 before it).
// (v7 = v2, rerun after the oracle rep-movs flag fix)
// FUNCTION: GP4 0x0054e13f
void __cdecl FUN_0054e13f(gp4::Regs* r) {
    const uint32_t shift = r->eax;
    const uint32_t cursor = r->edx;
    const uint32_t oldEnd = GP4_FIELD(uint32_t, r->edi, 4);
    GP4_FIELD(uint32_t, r->edi, 4) = oldEnd - shift;

    const uint32_t span = oldEnd - cursor + 1u;
    const uint32_t count = span - shift;
    uint32_t flags;
    if (static_cast<int32_t>(count) > 0) {
        uint8_t* target = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(cursor));
        const uint8_t* source = target + shift;
        for (uint32_t left = count; left != 0u; --left) {
            *target++ = *source++;
        }
        flags = gp4::flags::add32(cursor, shift);
    } else {
        gp4::cdecl_<0x0054e232, void>();
        flags = gp4::flags::sub32(span, shift);
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0054e13f, FUN_0054e13f, "regs(eax:i32[-8..8], edi:ptr[0x10]:layout=buf_0054e12a, edx:rec[0xa20000,1,0..64]) -> (cf, pf, af, zf, sf, of); globals=layout=text_0054e12a")
