#include <gp4/gp4.h>
#include <gp4/flags.h>

// For each of the 22 rows (index in [0x822278], 0..21) builds the per-place counts of 0x00548836 (places 1..22) into
// the 22 bytes at 0x831a98, then accumulates points: walking the counts with the points table at 0x7c3bb2, every
// nonzero count is clamped to the places still available (starting at the word [0x7c3bb0], 0xff when zero) and
// adds points * clamped-count; the total is stored as word number [0x822278] of the table at 0x7a93fa. The register
// state is the one the loops leave: eax = the last product (or the last points byte for a zero count), ebx/esi walk
// past both 22-byte tables, ecx = 0, edx = the places still unused, edi = the last word address and ebp = the last
// total. The flags are those of the final `cmp [0x822278], 0x16`.
// FUNCTION: GP4 0x0054857e
void __cdecl FUN_0054857e(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x00822278) = 0u;
    do {
        r->edi = 0x00831a98u;
        r->edx = GP4_GLOBAL(uint32_t, 0x00822278);
        r->ebx = 1u;
        do {
            gp4::call_regs(0x00548836, *r);
            GP4_GLOBAL(uint8_t, r->edi) = static_cast<uint8_t>(r->eax);
            r->edi += 1u;
            r->ebx += 1u;
        } while (r->ebx <= 0x16u);

        const uint8_t* counts = GP4_ARRAY(const uint8_t, 0x00831a98);
        const uint8_t* points = GP4_ARRAY(const uint8_t, 0x007c3bb2);
        uint32_t remaining = GP4_GLOBAL(uint16_t, 0x007c3bb0);
        if (remaining == 0u) {
            remaining = 0xffu;
        }
        uint32_t total = 0u;
        uint32_t product = 0u;
        for (uint32_t k = 0u; k != 0x16u; ++k) {
            uint32_t count = counts[k];
            product = points[k];
            if (count != 0u) {
                if (count > remaining) {
                    count = remaining;
                }
                product = product * count;
                total += product;
                remaining -= count;
            }
        }

        r->eax = product;
        r->ebx = 0x007c3bb2u + 0x16u;
        r->esi = 0x00831a98u + 0x16u;
        r->ecx = 0u;
        r->edx = remaining;
        r->ebp = total;
        r->edi = GP4_GLOBAL(uint32_t, 0x00822278) * 2u + 0x007a93fau;
        GP4_GLOBAL(uint16_t, r->edi) = static_cast<uint16_t>(total);
        GP4_GLOBAL(uint32_t, 0x00822278) += 1u;
    } while (GP4_GLOBAL(uint32_t, 0x00822278) < 0x16u);

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(GP4_GLOBAL(uint32_t, 0x00822278), 0x16u));
}
GP4_IMPL(0x0054857e, FUN_0054857e, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cnt_00548836")
