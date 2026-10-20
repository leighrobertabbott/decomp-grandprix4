#include <gp4/gp4.h>
#include <gp4/flags.h>

#pragma pack(push, 1)
// One entry of the 13-entry key table at 0x00533bc5 (in the code section): masked key word and
// the handler it runs. Keys are 0x400 | virtual-key (0x25 left, 0x27 right, 0x24 home, 0x23 end,
// 0x08 back, 0x2e delete, 0x1b escape, 0x0d return, 0x09 tab, ...).
struct KeyEntry_533bc5 {
    uint16_t Key;
    uint32_t Handler;
};
#pragma pack(pop)

// Text-field key handler (runs only when bit 0 of 0x0082fd6a is set, which it clears): the key word
// [0x007ab00e] & 0x6ff is looked up in the table 0x00533bc5 and a matching entry's handler runs
// (ebx = entry, ecx = entries left). Otherwise, when 0x00533d79 accepts the character (clear carry),
// it is typed into the buffer 0x0082811f: with the replace flag [0x00828101] = 0xff the field is
// first cleared down to its reserved suffix [0x008280f9] (0x00533dc5 removes the cursor glyph, the
// suffix moves to the start, cursor 0, the cursor glyph [0x00828161] is re-inserted); then, while
// the length [0x008280f5] - 1 is below the limit [0x008280f1], 0x00533d9e inserts the character and
// the cursor [0x008280ed] advances unless the width check ([0x00828107] set and eax + [0x00828113]
// >= [0x008280e5]) removes it again with 0x00533dc5. 0x0054d6f2 is an empty measure stub.
// FUNCTION: GP4 0x00533ae7
void __cdecl FUN_00533ae7(gp4::Regs* r) {
    const uint8_t gate = GP4_GLOBAL(uint8_t, 0x0082fd6a);
    if ((gate & 1u) == 0) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(gate & 1u));
        return;
    }
    GP4_GLOBAL(uint8_t, 0x0082fd6a) = static_cast<uint8_t>(gate & 0xfeu);

    const uint32_t key = GP4_GLOBAL(uint16_t, 0x007ab00e) & 0x6ffu;
    r->eax = (r->eax & 0xffff0000u) | key;

    const KeyEntry_533bc5* entry = reinterpret_cast<const KeyEntry_533bc5*>(0x00533bc5);
    uint32_t left = 0xdu;
    do {
        if (key == entry->Key) {
            r->ebx = reinterpret_cast<uint32_t>(entry);
            r->ecx = left;
            r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub16(key, entry->Key));
            gp4::call_regs(entry->Handler, *r);
            return;
        }
        ++entry;
    } while (--left != 0u);
    r->ebx = reinterpret_cast<uint32_t>(entry);
    r->ecx = 0u;

    gp4::call_regs(0x00533d79, *r);
    if ((r->eflags & gp4::CF) != 0u) {
        return;
    }

    const uint32_t typed = r->eax;
    if (GP4_GLOBAL(uint8_t, 0x00828101) == 0xffu) {
        gp4::call_regs(0x00533dc5, *r);
        const uint32_t suffix = GP4_GLOBAL(uint32_t, 0x008280f9);
        uint8_t* dst = reinterpret_cast<uint8_t*>(0x0082811f);
        const uint8_t* src = dst + (GP4_GLOBAL(uint32_t, 0x008280f5) - suffix);
        for (uint32_t count = suffix + 1u; count != 0u; --count) {
            *dst++ = *src++;
        }
        r->edi = reinterpret_cast<uint32_t>(dst);
        r->esi = reinterpret_cast<uint32_t>(src);
        r->ecx = 0u;
        const uint32_t kept = GP4_GLOBAL(uint32_t, 0x008280f9);
        GP4_GLOBAL(uint32_t, 0x008280f5) = kept;
        GP4_GLOBAL(uint32_t, 0x008280ed) = 0u;
        GP4_GLOBAL(uint8_t, 0x00828101) = 0u;
        r->eax = (kept & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x00828161);
        gp4::call_regs(0x00533d9e, *r);
        r->eax = typed;
    }

    const uint32_t last = GP4_GLOBAL(uint32_t, 0x008280f5) - 1u;
    r->ecx = last;
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x008280f1);
    if (last >= limit) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(last, limit));
        return;
    }
    gp4::call_regs(0x00533d9e, *r);

    uint32_t prev;
    if (GP4_GLOBAL(uint8_t, 0x00828107) == 0u) {
        prev = gp4::flags::sub8(0u, 0u);
    } else {
        r->esi = 0x0082811fu;
        gp4::cdecl_<0x0054d6f2, void>();
        const uint32_t width = r->eax + GP4_GLOBAL(uint32_t, 0x00828113);
        r->eax = width;
        const uint32_t room = GP4_GLOBAL(uint32_t, 0x008280e5);
        if (width >= room) {
            gp4::call_regs(0x00533dc5, *r);
            return;
        }
        prev = gp4::flags::sub32(width, room);
    }
    const uint32_t cursor = GP4_GLOBAL(uint32_t, 0x008280ed);
    GP4_GLOBAL(uint32_t, 0x008280ed) = cursor + 1u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::inc32(cursor, prev));
}
GP4_IMPL(0x00533ae7, FUN_00533ae7, "regs(eax:u32) -> (eax, ebx, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00533dc5+span_00533dc5+font_005339ed+chars_00828108+keys_00533ae7")
