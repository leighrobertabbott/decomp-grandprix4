#include <gp4/gp4.h>
#include <gp4/flags.h>

// Keyword lookup: ebx points at up to ten consecutive zero-terminated strings, esi at the text. The text
// is compared byte by byte with each entry; reaching an entry's terminator means a match (esi is then one
// past the byte compared with the terminator), a mismatch rewinds esi to the start and skips to the next
// entry. eax = the entry index (10 when nothing matched). Either exit leaves the flags of an equal compare
// (cmp ah,0 with ah = 0, or cmp ebp,10 with ebp = 10): ZF and PF set. edx and ebp are saved and restored.
// FUNCTION: GP4 0x005147f6
void __cdecl FUN_005147f6(gp4::Regs* r) {
    const uint8_t* start = reinterpret_cast<const uint8_t*>(r->esi);
    const uint8_t* text = start;
    const uint8_t* entry = reinterpret_cast<const uint8_t*>(r->ebx);

    uint32_t index = 0;
    for (;;) {
        const uint8_t ch = *text++;
        uint8_t want = *entry++;
        if (want == 0) {
            break;
        }
        if (ch == want) {
            continue;
        }
        text = start;
        do {
            want = *entry++;
        } while (want != 0);
        ++index;
        if (index == 10u) {
            break;
        }
    }

    r->eax = index;
    r->ebx = reinterpret_cast<uint32_t>(entry);
    r->esi = reinterpret_cast<uint32_t>(text);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(index, index));
}
GP4_IMPL(0x005147f6, FUN_005147f6, "regs(ebx:ptr[0x200]:u8{=0x41,0x42,0x43,0x00}, esi:ptr[0x40]:u8{=0x41,0x42,0x43,0x00}) -> (eax, ebx, esi, cf, pf, af, zf, sf, of)")
