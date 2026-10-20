#include <gp4/gp4.h>
#include <gp4/flags.h>

// Word-wraps the text of an edit record and trims it to at most five line breaks. edi is the text
// record (+0 text start, +4 end pointer E at the terminator, +8 cursor). Pass 1 walks the text from
// the start: each byte below 0x14 ends a line (0 ends the text); every other byte is a glyph code
// (0x14 -> 0x20, 0x15 -> 0x2d, 0x16..0x1f -> +0x1a) looked up in the font table [0x833f90] (dword
// offsets relative to the table, entry (code - [0x833f48])); the glyph's first byte is stored in
// [0x833f78], 0x0054d6f1 (a bare ret) runs, and the line room ([0x838c20] - [0x838c18] - 6) drops
// by width + glyph address + [0x833f6c]. When it goes below zero (signed), 0x0054e12a opens a
// one-byte gap at the glyph, a 0x0a break is written there (the cursor moves on when it lies after
// the gap) and a new line starts at the same glyph. [0x838c2a] counts the line ends.
// Pass 2: while there are 6 or more, the first line (through its end byte) is removed with
// 0x0054e13f and the cursor moves back by its length - unless the cursor lies inside that first line,
// in which case the last line (from the last end byte below E, signed byte compare) is removed once.
// The tail calls 0x0054e232 (a bare ret) inside pushal/popal when 6 or more remain; pushfd/popfd
// leave the flags of 'cmp dword [0x838c2a], 6'.
// FUNCTION: GP4 0x0054defe
void __cdecl FUN_0054defe(gp4::Regs* r) {
    uint8_t* const record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint32_t eax = r->eax;
    uint32_t ebx = r->ebx;
    uint32_t edx = r->edx;
    uint32_t esi = GP4_FIELD(uint32_t, record, 0);

    uint32_t& lines = GP4_GLOBAL(uint32_t, 0x00838c2a);
    lines = 0u;
    for (;;) {
        GP4_GLOBAL(uint8_t, 0x00833f74) = 0xffu;
        edx = GP4_GLOBAL(uint32_t, 0x00838c20) - GP4_GLOBAL(uint32_t, 0x00838c18) - 6u;
        eax = GP4_GLOBAL(uint32_t, 0x00838c18);
        uint8_t ch;
        for (;;) {
            ch = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(esi));
            eax = (eax & 0xffffff00u) | ch;
            if (ch < 0x14u) {
                break;
            }
            GP4_GLOBAL(uint8_t, 0x00833f76) = ch;
            uint32_t code = ch;
            if (code < 0x20u) {
                if (ch == 0x14u) {
                    code = 0x20u;
                } else if (ch == 0x15u) {
                    code = 0x2du;
                } else {
                    code += 0x1au;
                }
            }
            const uint32_t entry = (code - GP4_GLOBAL(uint32_t, 0x00833f48)) << 2;
            const uint32_t table = GP4_GLOBAL(uint32_t, 0x00833f90);
            ebx = table + *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(entry + table));
            const uint32_t width = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(ebx));
            GP4_GLOBAL(uint32_t, 0x00833f78) = width;
            gp4::cdecl_<0x0054d6f1, void>();
            eax = width + ebx + GP4_GLOBAL(uint32_t, 0x00833f6c);
            const bool overflow = static_cast<int32_t>(edx) < static_cast<int32_t>(eax);
            edx -= eax;
            if (overflow) {
                gp4::Regs gap = *r;
                gap.eax = 1u;
                gap.ebx = ebx;
                gap.edx = esi;
                gap.esi = esi;
                gp4::call_regs(0x0054e12a, gap);
                if (esi < GP4_FIELD(uint32_t, record, 8)) {
                    GP4_FIELD(uint32_t, record, 8) += 1u;
                }
                edx = esi;
                eax = 0x0au;
                ch = 0x0au;
                *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(esi)) = ch;
                break;
            }
            GP4_GLOBAL(uint8_t, 0x00833f75) = GP4_GLOBAL(uint8_t, 0x00833f76);
            ++esi;
        }
        if (ch == 0u) {
            break;
        }
        ++esi;
        ++lines;
    }

    while (lines >= 6u) {
        const uint32_t start = GP4_FIELD(uint32_t, record, 0);
        uint32_t p = start;
        uint8_t b;
        do {
            b = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(p));
            ++p;
        } while (b >= 0x14u);
        esi = p;
        if (static_cast<int32_t>(esi) < GP4_FIELD(int32_t, record, 8)) {
            const uint32_t length = esi - start;
            GP4_FIELD(uint32_t, record, 8) -= length;
            edx = start;
            eax = length;
            gp4::Regs cut = *r;
            cut.eax = eax;
            cut.ebx = ebx;
            cut.edx = edx;
            cut.esi = esi;
            gp4::call_regs(0x0054e13f, cut);
            --lines;
            continue;
        }
        const uint32_t end = GP4_FIELD(uint32_t, record, 4);
        esi = end;
        do {
            --esi;
        } while (*reinterpret_cast<const int8_t*>(static_cast<uintptr_t>(esi)) >= 0x14);
        edx = esi;
        eax = end - esi;
        gp4::Regs cut = *r;
        cut.eax = eax;
        cut.ebx = ebx;
        cut.edx = edx;
        cut.esi = esi;
        gp4::call_regs(0x0054e13f, cut);
        --lines;
        break;
    }

    const uint32_t count = lines;
    const uint32_t flags = gp4::flags::sub32(count, 6u);
    if (static_cast<int32_t>(count) >= 6) {
        gp4::cdecl_<0x0054e232, void>();
    }
    r->eax = eax;
    r->ebx = ebx;
    r->edx = edx;
    r->esi = esi;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0054defe, FUN_0054defe, "regs(edi:ptr[0x10]:layout=rec_0054defe, ebx:u32) -> (eax, ebx, edx, esi, cf, pf, af, zf, sf, of); globals=layout=text_0054defe")
