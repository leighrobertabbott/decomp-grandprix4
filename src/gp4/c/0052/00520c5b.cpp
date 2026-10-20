#include <gp4/gp4.h>
#include <gp4/flags.h>

// The original reads the third entry (edx = 2) of the lower-bound table 0x679b9c; the compiler would
// fold [edx*4 + base] into an absolute address and hide the table base.
static __declspec(noinline) int32_t tableEntry(const int32_t* base, int32_t index) {
    return base[index];
}

// Finds the game-state slot to restore/save: the mode word [0x679b44] becomes 2 and the slot rows (pitch
// [0x6975c4], base [0x7a8e18]) are scanned from the third lower bound [0x679ba4] for at most [0x7a8e1c]
// rows (`loop`) for a row whose first dword is not 1 and whose second dword equals the race clock
// [0x66d814]. Running out of rows leaves it at that; a row tagged 3 is loaded (0x00520b65 with eax = the
// row index), one tagged 2 is saved (0x00520b1b with eax = the row index), any other tag does nothing.
// eax ends as the last row tag (the row index after a call), edx as the last second dword (else the high
// half of index * pitch), ebp as the row index, esi as the row address, ecx as the remaining count; the
// flags are the last compare / the add of the row pitch, or those the callee leaves.
// FUNCTION: GP4 0x00520c5b
void __cdecl FUN_00520c5b(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x00679b44) = 2u;
    const uint32_t start = static_cast<uint32_t>(tableEntry(GP4_ARRAY(const int32_t, 0x00679b9c), 2));
    uint32_t ebp = start;
    const int64_t offset = static_cast<int64_t>(static_cast<int32_t>(start)) *
                           static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x006975c4));
    uint32_t eax = static_cast<uint32_t>(offset);
    uint32_t edx = static_cast<uint32_t>(static_cast<uint64_t>(offset) >> 32);
    uint32_t esi = GP4_GLOBAL(uint32_t, 0x007a8e18) + eax;
    uint32_t ecx = GP4_GLOBAL(uint32_t, 0x007a8e1c);
    uint32_t arith;

    for (;;) {
        const uint32_t* row = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(esi));
        eax = row[0];
        arith = gp4::flags::sub32(eax, 1u);
        if (eax != 1u) {
            edx = row[1];
            const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
            arith = gp4::flags::sub32(edx, clock);
            if (edx == clock) {
                break;
            }
        }
        ++ebp;
        const uint32_t pitch = GP4_GLOBAL(uint32_t, 0x006975c4);
        arith = gp4::flags::add32(esi, pitch);
        esi += pitch;
        --ecx;
        if (ecx == 0u) {
            r->eax = eax;
            r->ecx = ecx;
            r->edx = edx;
            r->esi = esi;
            r->ebp = ebp;
            r->eflags = gp4::flags::merge(r->eflags, arith);
            return;
        }
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = eax;
    c.ecx = ecx;
    c.edx = edx;
    c.esi = esi;
    c.ebp = ebp;
    arith = gp4::flags::sub32(eax, 3u);
    if (eax == 3u) {
        c.eax = ebp;
        c.eflags = gp4::flags::merge(c.eflags, arith);
        gp4::call_regs(0x00520b65, c);
        arith = c.eflags;
    } else {
        arith = gp4::flags::sub32(eax, 2u);
        if (eax == 2u) {
            c.eax = ebp;
            c.eflags = gp4::flags::merge(c.eflags, arith);
            gp4::call_regs(0x00520b1b, c);
            arith = c.eflags;
        }
    }

    r->eax = c.eax;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00520c5b, FUN_00520c5b, "regs() -> (eax, ecx, edx, esi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slot_00520b1b+copy_0063b814+links_0066fef0+ptrs_0066d818+ring_008943a0+records_008943a0+segw_00441e25+words_008943a0+ptr_0086850c+idx_00520b65+rows_00520c5b")
