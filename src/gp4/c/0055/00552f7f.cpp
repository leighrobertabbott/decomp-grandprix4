#include <gp4/gp4.h>
#include <gp4/flags.h>

// One link of the chain: tests `mask` in the byte +0x76 of the record esi. A clear bit moves on to the
// next link (returns false). A set bit copies the source record `table` into object 10 through
// 0x00552cfa; a non-zero result (ZF clear) ends the chain, otherwise the state dword 0x00866dc4 is set
// to `next`; a carry ends the chain; then the budget dword 0x00866e10 is decremented and a negative
// result ends it. Returns true when the chain ends.
template <uint32_t Mask, uint32_t Table, uint32_t Next>
static inline bool ChainLink(gp4::Regs& c) {
    const uint32_t bits = GP4_FIELD(uint8_t, c.esi, 0x76) & Mask;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(bits));
    if (bits == 0u) {
        return false;
    }
    c.eax = 0xau;
    c.edx = Table;
    gp4::call_regs(0x00552cfa, c);
    if ((c.eflags & gp4::flags::ZF) == 0u) {
        return true;
    }
    GP4_GLOBAL(uint32_t, 0x00866dc4) = Next;
    if ((c.eflags & gp4::flags::CF) != 0u) {
        return true;
    }
    const uint32_t budget = GP4_GLOBAL(uint32_t, 0x00866e10);
    GP4_GLOBAL(uint32_t, 0x00866e10) = budget - 1u;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::dec32(budget, c.eflags));
    return (c.eflags & gp4::flags::SF) != 0u;
}

// Round-robin sender over four source records (state dword 0x00866dc4 = the first link to try, values
// above 3 count as 3; eax holds the jump-table address of that link until a call overwrites it). Link
// k is armed by bit 0x80 >> k of the byte +0x76 of the record esi; each armed link copies its source
// record 0x00865f58 + 0x10 * k into object 10 (0x00552cfa) and advances the state to k + 1 (mod 4).
// Registers and flags are whatever the last link leaves.
// FUNCTION: GP4 0x00552f7f
void __cdecl FUN_00552f7f(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    const uint32_t state = GP4_GLOBAL(uint32_t, 0x00866dc4);
    const uint32_t index = state < 4u ? state : 3u;
    c.eax = (index << 2) + 0x00552f9cu;
    switch (index) {
    case 0:
        if (ChainLink<0x80u, 0x00865f58u, 1u>(c)) {
            break;
        }
    case 1:
        if (ChainLink<0x40u, 0x00865f68u, 2u>(c)) {
            break;
        }
    case 2:
        if (ChainLink<0x20u, 0x00865f78u, 3u>(c)) {
            break;
        }
    default:
        ChainLink<0x10u, 0x00865f88u, 0u>(c);
        break;
    }
    *r = c;
}
GP4_IMPL(0x00552f7f, FUN_00552f7f, "regs(esi:ptr[0xe0]:layout=fxowner_0055278a) -> (eax, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=tbl_00854460+trackgeo_008943a0+tbl_00868598+state_0040705f+state_00407af6+state_0040759e+trackfar_008943a0+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+pit_00406dbb+segflag_004381ab+w6_008943a0+fx_00866248+chain_00552cfa")
