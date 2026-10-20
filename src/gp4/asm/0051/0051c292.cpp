#include <gp4/gp4.h>
#include <gp4/flags.h>

// A 0x20-byte entry of the texture table [0x6975dc] (the same records 0x0051c007 and 0x0051c21b walk).
#pragma pack(push, 1)
struct Unknown_Rec20 {
    uint32_t Unknown00;   // low byte is the key 0x0051c21b sorts on
    uint8_t Unknown04[8];
    uint32_t Offset0C;    // texel offset into the stream data, rewritten to the packed address
    uint16_t Width10;     // bytes per row (4 rows)
    uint8_t Unknown12[2];
    uint16_t Flags14;
    uint8_t Unknown16[4];
    uint16_t Flags1A;
    uint8_t Unknown1C;
    uint8_t Unknown1D;
    uint8_t Unknown1E[2];
};
#pragma pack(pop)

// FUNCTION: GP4 0x0051c292
// pushal wrapper. Unless the first entry of [0x6975dc] has bit 4 in word +0x1a or +0x14, it clears the
// [0x96a368] slot bytes at 0x965914, then repeatedly asks 0x0051c21b for the entry whose key is nearest
// above the current column ((out - [0x6975e0]) & 0xff); it stops when that reports carry. Each chosen
// entry's 4 rows of width +0x10 are copied from [0x965908] + offset +0xc to out + distance (rows 0x100
// apart), +0xc becomes that address and out moves 0x400 further; out + 0x400 above [0x7a8e36] + [0x7a8e3a]
// hangs (jmp $). [0x6975e0] then becomes out rounded up to 0x100 relative to [0x7a8e36]. A second pass
// over the [0x96a368] entries rebases dword +0 by [0x6975e0] (minus 0x10000 for packed entries, whose
// byte +0x1d becomes bits 8..15 of +0xc - +0) and [0x6975dc] advances past them. popal restores every
// GPR; the flags are those of the final `dec ebp` (1 -> 0) with CF from the last `add ebx, 0x20`.
void __cdecl FUN_0051c292(gp4::Regs* r) {
    const Unknown_Rec20* first =
        reinterpret_cast<const Unknown_Rec20*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x006975dc)));
    if ((first->Flags1A & 4u) == 0u && (first->Flags14 & 4u) == 0u) {
        uint32_t remaining = GP4_GLOBAL(uint32_t, 0x0096a368);
        uint8_t* slot = GP4_ARRAY(uint8_t, 0x00965914);
        for (uint32_t n = remaining; n != 0u; --n) {
            *slot++ = 0u;
        }
        uint32_t out = GP4_GLOBAL(uint32_t, 0x006975e0);
        gp4::Regs c = *r;
        do {
            c.eax = (out - GP4_GLOBAL(uint32_t, 0x006975e0)) & 0xffu;
            c.edi = out;
            c.ebp = remaining;
            gp4::call_regs(0x0051c21b, c);
            if ((c.eflags & gp4::flags::CF) != 0u) {
                break;
            }
            out += c.eax;
            if (out + 0x400u > GP4_GLOBAL(uint32_t, 0x007a8e36) + GP4_GLOBAL(uint32_t, 0x007a8e3a)) {
                for (;;) {
                }
            }
            Unknown_Rec20* rec = reinterpret_cast<Unknown_Rec20*>(static_cast<uintptr_t>(c.ebx));
            const uint32_t width = rec->Width10;
            const uint8_t* src =
                reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(rec->Offset0C + GP4_GLOBAL(uint32_t, 0x00965908)));
            rec->Offset0C = out;
            for (int row = 0; row < 4; ++row) {
                uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(out));
                for (uint32_t n = width; n != 0u; --n) {
                    *dst++ = *src++;
                }
                out += 0x100u;
            }
            --remaining;
        } while (remaining != 0u);
        const uint32_t base = GP4_GLOBAL(uint32_t, 0x007a8e36);
        GP4_GLOBAL(uint32_t, 0x006975e0) = ((out - base + 0xffu) & 0xffffff00u) + base;
    }

    uint32_t remaining = GP4_GLOBAL(uint32_t, 0x0096a368);
    Unknown_Rec20* rec = reinterpret_cast<Unknown_Rec20*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x006975dc)));
    uint32_t advance;
    uint32_t before;
    do {
        if ((rec->Flags1A & 4u) != 0u || (rec->Flags14 & 4u) != 0u || (rec->Flags14 & 0x80u) != 0u) {
            rec->Offset0C = GP4_GLOBAL(uint32_t, 0x006975e0);
            rec->Unknown00 = GP4_GLOBAL(uint32_t, 0x006975e0) + rec->Unknown00;
            rec->Unknown1D = 0u;
        } else {
            const uint32_t pos = GP4_GLOBAL(uint32_t, 0x006975e0) - 0x10000u + rec->Unknown00;
            rec->Unknown00 = pos;
            rec->Unknown1D = static_cast<uint8_t>((rec->Offset0C - pos) >> 8);
        }
        advance = gp4::flags::add32(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(rec)), 0x20u);
        rec += 1;
        before = remaining;
        --remaining;
    } while (remaining != 0u);
    GP4_GLOBAL(uint32_t, 0x006975dc) = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(rec));
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(before, advance));
}
GP4_IMPL(0x0051c292, FUN_0051c292, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=count_0096a368+recs_0051c21b+pack_0051c292")
