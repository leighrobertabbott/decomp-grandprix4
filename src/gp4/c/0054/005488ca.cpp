#include <gp4/gp4.h>
#include <gp4/flags.h>

// Builds the 11-row (score sum, best score) table at 0x00831ab9 and the row order at 0x00831aae.
// The 22 bytes at 0x007c31dc are driver numbers with flag bits (low six bits - 1 = the driver index);
// a driver counts when its time dword 0x007c3208 + 4 * index is neither the no-time marker 0x80000000
// nor <= -1000. Its row is 0x005484eb(index) (the 0x007c2a48 slot lookup, halved); the next byte of
// 0x007c3bb2 is added to the row's first byte and raised into its second byte (a running maximum).
// The order bytes 0x00831aae.. get 0..10, then the rows are bubble-sorted descending by (first,
// second) byte, swapping the order bytes along. Registers are left as the loops leave them: ecx = 0,
// esi = 0x00831acd, edi = 0x00831ab8, edx = the last time dword loaded (low byte cleared by the final
// pass), eax / ebx from the last compare / swap, and the flags are those of the final `cmp dl, 0`.
// FUNCTION: GP4 0x005488ca
void __cdecl FUN_005488ca(gp4::Regs* r) {
    uint8_t* const rows = GP4_ARRAY(uint8_t, 0x00831ab9);
    uint8_t* const order = GP4_ARRAY(uint8_t, 0x00831aae);
    const uint8_t* const drivers = GP4_ARRAY(const uint8_t, 0x007c31dc);
    const uint32_t* const times = GP4_ARRAY(const uint32_t, 0x007c3208);
    const uint8_t* score = GP4_ARRAY(const uint8_t, 0x007c3bb2);

    for (uint32_t i = 0; i < 0x16u; ++i) {
        rows[i] = 0u;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t edi = 0x00831acfu;

    for (uint32_t left = 0x16u; left != 0u; --left) {
        const uint32_t index = (static_cast<uint32_t>(drivers[0x16u - left]) & 0x3fu) - 1u;
        eax = index;
        if (index < 0x16u) {
            edx = times[index];
            if (edx != 0x80000000u && static_cast<int32_t>(edx) > -1000) {
                c.eax = index;
                c.ecx = left;
                c.edx = edx;
                c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(score));
                c.edi = edi;
                c.ebx = 0x007c31dcu + (0x16u - left) + 1u;
                gp4::call_regs(0x005484eb, c);

                const uint32_t row = c.eax << 1;
                edi = row + 0x00831ab9u;
                const uint8_t value = *score;
                ++score;
                eax = value;
                uint8_t* const cell = rows + row;
                cell[0] = static_cast<uint8_t>(cell[0] + value);
                if (value >= cell[1]) {
                    cell[1] = value;
                }
            }
        }
    }

    for (uint32_t i = 0; i < 0xbu; ++i) {
        order[i] = static_cast<uint8_t>(i);
    }
    eax = (eax & 0xffffff00u) | 0xbu;

    uint32_t ebx = 0x007c31dcu + 0x16u;
    bool swapped;
    do {
        swapped = false;
        for (uint32_t i = 0; i < 10u; ++i) {
            uint8_t* const p = rows + 2u * i;
            uint8_t* const o = order + i;
            uint32_t first = p[0];
            bool swap;
            if (first < p[2]) {
                swap = true;
            } else if (first != p[2]) {
                swap = false;
            } else {
                first = p[1];
                swap = first < p[3];
            }
            eax = (eax & 0xffffff00u) | first;
            if (swap) {
                eax = (eax & 0xffff0000u) | (static_cast<uint32_t>(p[1]) << 8) | o[0];
                ebx = (ebx & 0xffff0000u) | (static_cast<uint32_t>(p[3]) << 8) | o[1];
                const uint8_t w0 = p[0];
                const uint8_t w1 = p[1];
                p[0] = p[2];
                p[1] = p[3];
                p[2] = w0;
                p[3] = w1;
                const uint8_t saved = o[0];
                o[0] = o[1];
                o[1] = saved;
                swapped = true;
            }
        }
    } while (swapped);

    r->eax = eax;
    r->ebx = ebx;
    r->ecx = 0u;
    r->edx = edx & 0xffffff00u;
    r->esi = 0x00831acdu;
    r->edi = 0x00831ab8u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(0u, 0u));
}
GP4_IMPL(0x005488ca, FUN_005488ca, "regs(edx:u32) -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=order_00532396+pairs_005487cb")
