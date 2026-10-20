#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sorts the 22 bytes at 0x7a943c (initialised 0..21) in descending order of the 15-bit scores at 0x7a93fa[index];
// equal nonzero scores are ordered by the place counts of 0x00548836 for places 1..22 (more first). A bubble sort
// repeated until a pass swaps nothing. Register state left by the last comparison of the last pass: esi/edi hold the
// compared indices (doubled word offsets, halved again after a tie), eax the last score or count (low word replaced
// by the swapped bytes), ebx walks the table (end - 1), ecx = 0 and dl = 0; the flags are those of `or dl, dl`.
// FUNCTION: GP4 0x00548602
void __cdecl FUN_00548602(gp4::Regs* r) {
    uint8_t* const order = GP4_ARRAY(uint8_t, 0x007a943c);
    const uint16_t* const scores = GP4_ARRAY(const uint16_t, 0x007a93fa);
    uint32_t eax = r->eax;
    uint32_t esi = r->esi;
    uint32_t edi = r->edi;
    uint32_t dl;

    for (uint32_t i = 0u; i != 0x16u; ++i) {
        order[i] = static_cast<uint8_t>(i);
    }
    do {
        dl = 0u;
        uint8_t* entry = order;
        for (uint32_t left = 0x15u; left != 0u; --left, ++entry) {
            esi = static_cast<uint32_t>(entry[0]) << 1;
            edi = static_cast<uint32_t>(entry[1]) << 1;
            eax = scores[entry[0]] & 0x7fffu;
            const uint32_t other = scores[entry[1]] & 0x7fffu;
            bool swap = false;
            if (eax > other) {
                swap = false;
            } else if (eax != other) {
                swap = true;
            } else if (eax != 0u) {
                esi >>= 1;
                edi >>= 1;
                for (uint32_t place = 1u; place <= 0x16u; ++place) {
                    gp4::Regs c = *r;
                    c.ebx = place;
                    c.edx = edi;
                    gp4::call_regs(0x00548836, c);
                    const uint32_t second = c.eax;
                    c.edx = esi;
                    gp4::call_regs(0x00548836, c);
                    eax = c.eax;
                    if (eax > second) {
                        break;
                    }
                    if (eax != second) {
                        swap = true;
                        break;
                    }
                }
            }
            if (swap) {
                eax = (eax & 0xffff0000u) | entry[0] | (static_cast<uint32_t>(entry[1]) << 8);
                const uint8_t first = entry[0];
                entry[0] = entry[1];
                entry[1] = first;
                dl = 0xffu;
            }
        }
    } while (dl != 0u);

    r->eax = eax;
    r->ebx = 0x007a943cu + 0x15u;
    r->ecx = 0u;
    r->edx = r->edx & 0xffffff00u;
    r->esi = esi;
    r->edi = edi;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(dl));
}
GP4_IMPL(0x00548602, FUN_00548602, "regs(edx:u32) -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=words_0054888e+cnt_00548836")
