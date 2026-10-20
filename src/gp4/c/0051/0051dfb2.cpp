#include <gp4/gp4.h>
#include <gp4/flags.h>

// Control-record routine (esi = record from 0x0066d820, called by 0x0051d856 after 0x0051de03).
// The mode byte 0x007ab550 is left in bl.
//   mode 1      -> bit 0 of +0x6f set unless bit 0 of byte 0x00679d1b is set.
//   other modes -> 0x0051ddda tests the mode's flag bit: set -> +0x6f bit 0; clear -> nothing;
//                  no flag bit (CF) -> axis modes 6..9 / 0x20..0x27 clamp the position 0x007ab15c
//                  between the end stops 0x007ab144 and 0x007ab140 (either order), map it to
//                  (pos - [0x007ab144]) * 0x4000 / ([0x007ab140] - [0x007ab144]) (zero range
//                  divides by 1), clamp |q| to 0x3f80 and store q >> 7 (sar) at +0x6e; a value of
//                  0x40..0x7f also sets +0x6f bit 0. eax/ecx/edx keep the working values.
// FUNCTION: GP4 0x0051dfb2
void __cdecl FUN_0051dfb2(gp4::Regs* r) {
    uint8_t* item = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007ab550);
    r->ebx = (r->ebx & 0xffffff00u) | mode;

    if (mode == 1u) {
        const uint32_t bit = GP4_GLOBAL(uint8_t, 0x00679d1b) & 1u;
        if (bit != 0u) {
            r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(bit));
            return;
        }
        const uint8_t b = static_cast<uint8_t>(item[0x6f] | 1u);
        item[0x6f] = b;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(b));
        return;
    }

    gp4::Regs c = *r;
    gp4::call_regs(0x0051ddda, c);
    r->eax = c.eax;
    r->eflags = c.eflags;
    if ((c.eflags & gp4::flags::CF) == 0u) {
        if ((c.eflags & gp4::flags::ZF) == 0u) {
            const uint8_t b = static_cast<uint8_t>(item[0x6f] | 1u);
            item[0x6f] = b;
            r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(b));
        }
        return;
    }

    bool axis;
    switch (mode) {
    case 6: case 7: case 0x20: case 0x22: case 0x23: case 0x24:
    case 8: case 9: case 0x21: case 0x25: case 0x26: case 0x27:
        axis = true;
        break;
    default:
        axis = false;
        break;
    }
    if (!axis) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(mode, 0x27u));
        return;
    }

    const int32_t first = GP4_GLOBAL(int32_t, 0x007ab140);
    const int32_t second = GP4_GLOBAL(int32_t, 0x007ab144);
    int32_t position = GP4_GLOBAL(int32_t, 0x007ab15c);
    uint32_t range = static_cast<uint32_t>(first) - static_cast<uint32_t>(second);
    if (static_cast<int32_t>(range) < 0) {
        if (position < first) {
            position = first;
        }
        if (position > second) {
            position = second;
        }
    } else {
        if (position < second) {
            position = second;
        }
        if (position > first) {
            position = first;
        }
    }
    if (range == 0u) {
        range = 1u;
    }
    const int32_t dividend =
        static_cast<int32_t>((static_cast<uint32_t>(position) - static_cast<uint32_t>(second)) << 14);
    int32_t remainder = 0;
    uint32_t level = static_cast<uint32_t>(
        gp4::idiv64(static_cast<int64_t>(dividend), static_cast<int32_t>(range), &remainder));

    if (static_cast<int32_t>(level) < 0) {
        uint32_t magnitude = 0u - level;
        if (magnitude >= 0x3f80u) {
            magnitude = 0x3f80u;
        }
        level = 0u - magnitude;
    } else if (level >= 0x3f80u) {
        level = 0x3f80u;
    }
    const uint32_t result = static_cast<uint32_t>(static_cast<int32_t>(level) >> 7);
    const uint8_t al = static_cast<uint8_t>(result);
    item[0x6e] = al;

    r->eax = result;
    r->ecx = range;
    r->edx = static_cast<uint32_t>(remainder);

    uint32_t fl;
    if ((al & 0x80u) != 0u) {
        fl = gp4::flags::logic8(al);
    } else if (static_cast<int8_t>(al) < 0x40) {
        fl = gp4::flags::sub8(al, 0x40u);
    } else {
        const uint8_t b = static_cast<uint8_t>(item[0x6f] | 1u);
        item[0x6f] = b;
        fl = gp4::flags::logic8(b);
    }
    r->eflags = gp4::flags::merge(r->eflags, fl);
}
GP4_IMPL(0x0051dfb2, FUN_0051dfb2, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:ptr[0x78]:bytes) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=bits_006f2d90+axis_0051dfb2")
