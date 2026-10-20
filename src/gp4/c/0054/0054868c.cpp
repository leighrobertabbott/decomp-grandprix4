#include <gp4/gp4.h>
#include <gp4/flags.h>

// Tie marker pass over the 21 consecutive pairs (a, b) of the byte list 0x007a943c (indices into the
// 22 score words at 0x007a93fa). When the two words agree in their low 15 bits and are nonzero, the
// finishing-place counts of the two entries (0x00548836 with bl = 1..0x1a, row = the entry index)
// are compared; if all 26 counts match, bit 15 of entry b's word is set. Leaves ebx = list end,
// ecx = 0 and eax / edx / esi / edi as the last pair left them; the flags are those of the final
// `inc ebx` (CF kept from the previous flag writer).
// FUNCTION: GP4 0x0054868c
void __cdecl FUN_0054868c(gp4::Regs* r) {
    const uint8_t* const pairs = GP4_ARRAY(uint8_t, 0x007a943c);
    uint16_t* const words = GP4_ARRAY(uint16_t, 0x007a93fa);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    uint32_t eax = 0u;
    uint32_t edx = 0u;
    uint32_t esi = 0u;
    uint32_t edi = 0u;
    uint32_t arith = 0u;

    for (uint32_t i = 0u; i < 0x15u; ++i) {
        esi = static_cast<uint32_t>(pairs[i]) << 1;
        edi = static_cast<uint32_t>(pairs[i + 1u]) << 1;
        eax = words[esi >> 1] & 0x7fffu;
        edx = words[edi >> 1] & 0x7fffu;
        arith = gp4::flags::logic32(edx);
        arith = gp4::flags::sub32(eax, edx);
        if (eax == edx) {
            arith = gp4::flags::logic32(eax);
            if (eax != 0u) {
                esi >>= 1;
                edi >>= 1;
                arith = gp4::flags::logic32(edi);
                bool tied = true;
                uint32_t place = 1u;
                do {
                    c.eax = eax;
                    c.ebx = place;
                    c.edx = edi;
                    gp4::call_regs(0x00548836, c);
                    const uint32_t first = c.eax;
                    arith = c.eflags;
                    c.edx = esi;
                    gp4::call_regs(0x00548836, c);
                    eax = c.eax;
                    arith = c.eflags;
                    edx = first;
                    arith = gp4::flags::sub32(eax, edx);
                    if (eax != edx) {
                        tied = false;
                        break;
                    }
                    arith = gp4::flags::inc32(place, arith);
                    ++place;
                    arith = gp4::flags::sub32(place, 0x1bu);
                } while (place < 0x1bu);
                if (tied) {
                    edi <<= 1;
                    arith = gp4::flags::logic32(edi);
                    const uint16_t marked = static_cast<uint16_t>(words[edi >> 1] | 0x8000u);
                    words[edi >> 1] = marked;
                    arith = gp4::flags::logic16(marked);
                }
            }
        }
        arith = gp4::flags::inc32(0x007a943cu + i, arith);
    }

    r->eax = eax;
    r->ebx = 0x007a943cu + 0x15u;
    r->ecx = 0u;
    r->edx = edx;
    r->esi = esi;
    r->edi = edi;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0054868c, FUN_0054868c, "regs() -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=words_0054888e+cnt_00548836+idx_0054868c")
