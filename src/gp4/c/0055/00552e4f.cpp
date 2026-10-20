#include <gp4/gp4.h>
#include <gp4/flags.h>

// One link of the chain: tests `mask` in the byte +0x76 of the record esi. A clear bit moves on to the
// next link (returns false). A set bit copies the source record `table` into object 6 through
// 0x00552cfa; a non-zero result (ZF clear) ends the chain, otherwise the state dword 0x00866dcc is set
// to `next`; a carry ends the chain; then the budget dword 0x00866e10 is decremented and a negative
// result ends it. Returns true when the chain ends.
template <uint32_t Mask, uint32_t Table, uint32_t Next>
static inline bool ChainLink(gp4::Regs& c) {
    const uint32_t bits = GP4_FIELD(uint8_t, c.esi, 0x76) & Mask;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(bits));
    if (bits == 0u) {
        return false;
    }
    c.eax = 6u;
    c.edx = Table;
    gp4::call_regs(0x00552cfa, c);
    if ((c.eflags & gp4::flags::ZF) == 0u) {
        return true;
    }
    GP4_GLOBAL(uint32_t, 0x00866dcc) = Next;
    if ((c.eflags & gp4::flags::CF) != 0u) {
        return true;
    }
    const uint32_t budget = GP4_GLOBAL(uint32_t, 0x00866e10);
    GP4_GLOBAL(uint32_t, 0x00866e10) = budget - 1u;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::dec32(budget, c.eflags));
    return (c.eflags & gp4::flags::SF) != 0u;
}

// Kept out of line so the object-6 limit 0x008662b4 + 6 * 0xe0 stays a displacement from the index.
static __declspec(noinline) int32_t ObjectLimit(uint32_t byteOffset) {
    return GP4_FIELD(int32_t, 0x008662b4u, byteOffset);
}

// Repeating round-robin sender (twin of 0x00552d1e for object 6, state dword 0x00866dcc, link bits
// 8/4/2/1 of the byte +0x76 of the record esi, budget 0x00866e10 = 100): runs the four links starting
// at the one the state names (values above 3 count as 3), then repeats from link 0 while any of the
// bits 0x0f is set and the clock dword 0x00866db8 has not fallen below the limit of object 6 (+0x540
// from 0x008662b4). Registers and flags are what the last step leaves; the loop test leaves
// eax = 0x540, edx = the clock and the flags of the `cmp`.
// FUNCTION: GP4 0x00552e4f
void __cdecl FUN_00552e4f(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    GP4_GLOBAL(uint32_t, 0x00866e10) = 0x64u;
    const uint32_t state = GP4_GLOBAL(uint32_t, 0x00866dcc);
    uint32_t index = state < 4u ? state : 3u;
    c.eax = (index << 2) + 0x00552e74u;
    for (;;) {
        switch (index) {
        case 0:
            if (ChainLink<0x08u, 0x00865f58u, 1u>(c)) {
                goto done;
            }
        case 1:
            if (ChainLink<0x04u, 0x00865f68u, 2u>(c)) {
                goto done;
            }
        case 2:
            if (ChainLink<0x02u, 0x00865f78u, 3u>(c)) {
                goto done;
            }
        default:
            if (ChainLink<0x01u, 0x00865f88u, 0u>(c)) {
                goto done;
            }
        }

        // The four `test byte [esi+0x76], mask` jumps all lead on once any bit is set; with all four
        // clear the function ends on the flags of the last (0x01) test.
        const uint32_t bits = GP4_FIELD(uint8_t, c.esi, 0x76) & 0x0fu;
        if (bits == 0u) {
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(0u));
            goto done;
        }
        c.eax = 6u * 0xe0u;
        c.edx = GP4_GLOBAL(uint32_t, 0x00866db8);
        const int32_t limit = ObjectLimit(c.eax);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.edx, static_cast<uint32_t>(limit)));
        if (static_cast<int32_t>(c.edx) < limit) {
            goto done;
        }
        index = 0u;
    }
done:
    *r = c;
}
GP4_IMPL(0x00552e4f, FUN_00552e4f, "regs(esi:ptr[0xe0]:layout=fxowner_0055278a) -> (eax, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=tbl_00854460+trackgeo_008943a0+tbl_00868598+state_0040705f+state_00407af6+state_0040759e+trackfar_008943a0+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+pit_00406dbb+segflag_004381ab+w6_008943a0+fx_00866248+chain_00552cfa")
