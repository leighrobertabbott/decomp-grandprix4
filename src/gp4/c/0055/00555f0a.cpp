#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00555f0a
// Stream-record decoder step (esi = record, ebx = the 0x160-byte ring record it belongs to). In mode
// 2 ([0x86857c]) it packs the record fields into one dword -- byte +6 | byte +8 << 8 |
// ((w+0xa & 0x30) << 2 | (w+0x10 & 1) << 5 | w+0xc & 0x1f) << 16 | byte(w+0xe) << 24 -- and appends
// it (with the word at +4) through 0x555ea4 to the chain of ring record ebx. The word at +0 is a
// repeat count (at least one append): after each further append ebx advances by max(w+2, 1) ring
// records, wrapping by [0x866ebc] at the ring end [0x866eb8]. pushal/popal keep every register but
// esi, which both paths advance by 0x12; the flags are those of that final `add esi, 0x12`.
void __cdecl FUN_00555f0a(gp4::Regs* r) {
    const uint32_t esi = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) == 2u) {
        uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(esi));
        uint32_t repeat = GP4_FIELD(uint16_t, record, 0);
        uint32_t advance = GP4_FIELD(uint16_t, record, 2);
        if (static_cast<int32_t>(advance) < 1) {
            advance = 1u;
        }

        uint32_t low = (GP4_FIELD(uint16_t, record, 0x0a) & 0x30u) << 2;
        low |= (GP4_FIELD(uint16_t, record, 0x10) & 1u) << 5;
        low |= GP4_FIELD(uint16_t, record, 0x0c) & 0x1fu;
        const uint32_t high = GP4_FIELD(uint16_t, record, 0x0e) & 0xffu;
        const uint32_t value = (high << 24) | ((low & 0xffu) << 16) |
                               (static_cast<uint32_t>(GP4_FIELD(uint8_t, record, 8)) << 8) |
                               GP4_FIELD(uint8_t, record, 6);

        gp4::Regs c = *r;
        c.eax = value;
        c.esi = (esi & 0xffff0000u) | GP4_FIELD(uint16_t, record, 4);
        for (;;) {
            gp4::call_regs(0x00555ea4, c);
            if (repeat == 0u) {
                break;
            }
            repeat -= 1u;
            if (repeat == 0u) {
                break;
            }
            uint32_t n = advance;
            do {
                c.ebx += 0x160u;
                if (!(c.ebx < GP4_GLOBAL(uint32_t, 0x00866eb8))) {
                    c.ebx -= GP4_GLOBAL(uint32_t, 0x00866ebc);
                }
                n -= 1u;
            } while (n != 0u);
        }
    }
    r->esi = esi + 0x12u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi, 0x12u));
}
GP4_IMPL(0x00555f0a, FUN_00555f0a, "regs(ebx:rec[0x8943a0,0x160,0..29], esi:ptr[0x20]:layout=rec_00555f0a) -> (esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+mode_0086857c+slots_00555f0a")
