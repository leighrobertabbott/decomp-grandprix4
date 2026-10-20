#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00557903
// Sibling of 0x00557843 that also clears the dword mask: in mode 2 ([0x86857c]; otherwise only
// esi += 2) the ring records after the high-water mark [0x871ca4] up to (excluding) the record ebx
// get the pending dword mask [0x8705f4] or-ed into +0x2c and the pending byte mask [0x87062f] into
// +0x67, the mark following each. The byte mask then loses the bits of the stream word at [esi],
// esi advances by 2 and 0x5577c5 turns the word into status bits (edx), which are cleared from the
// dword mask (and with ~edx). eax ends as the dword mask with its low word replaced by the stream
// word, edx as ~bits; edi is pushed/popped; the flags are those of the final `and [0x8705f4], edx`.
void __cdecl FUN_00557903(gp4::Regs* r) {
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 2u) {
        const uint32_t before = r->esi;
        r->esi = before + 2u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(before, 2u));
        return;
    }

    uint32_t record = GP4_GLOBAL(uint32_t, 0x00871ca4) + 0x160u;
    const uint32_t dwordMask = GP4_GLOBAL(uint32_t, 0x008705f4);
    const uint8_t byteMask = GP4_GLOBAL(uint8_t, 0x0087062f);
    if (record < r->ebx) {
        do {
            uint8_t* p = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(record));
            GP4_FIELD(uint32_t, p, 0x2c) |= dwordMask;
            GP4_FIELD(uint8_t, p, 0x67) |= byteMask;
            GP4_GLOBAL(uint32_t, 0x00871ca4) = record;
            record += 0x160u;
        } while (record != r->ebx);
    }

    const uint32_t word = GP4_GLOBAL(uint16_t, r->esi);
    GP4_GLOBAL(uint8_t, 0x0087062f) &= static_cast<uint8_t>(~word);

    gp4::Regs c = *r;
    c.eax = (dwordMask & 0xffff0000u) | word;
    c.edx = (r->edx & 0xffffff00u) | byteMask;
    c.edi = record;
    c.esi = r->esi + 2u;
    gp4::call_regs(0x005577c5, c);

    const uint32_t cleared = ~c.edx;
    GP4_GLOBAL(uint32_t, 0x008705f4) &= cleared;

    r->eax = c.eax;
    r->edx = cleared;
    r->esi = c.esi;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(GP4_GLOBAL(uint32_t, 0x008705f4)));
}
GP4_IMPL(0x00557903, FUN_00557903, "regs(ebx:rec[0x8943a0,0x160,0..29], esi:ptr[8]:bytes) -> (eax, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+mode_0086857c+mark_005578a4")
