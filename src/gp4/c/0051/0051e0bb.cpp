#include <gp4/gp4.h>
#include <gp4/flags.h>

// Register routine (esi = control record from 0x0066d820), sibling of 0x0051e1c4. Mode byte
// 0x007ab553 (byte +0xb of the control block 0x7ab548, also left in bl):
//   1      -> sets bit 1 of +0x6f unless bit 1 of the byte 0x00679d1b is set.
//   other  -> 0x0051ddda tests the mode's flag bit: set -> bit 1 of +0x6f; clear -> nothing;
//             no bit (CF) and an axis code (6..9, 0x20..0x27) -> the position 0x007ab160 is
//             clamped between the end stops 0x007ab14c and 0x007ab148 (either order), mapped to
//             (pos - [0x7ab14c]) * 0x4000 / ([0x7ab148] - [0x7ab14c]) (zero range divides by 1),
//             clamped by magnitude to 0x3f80, stored >> 7 (sar) at +0x6c; a level of 0x40 or
//             more sets bit 1 of +0x6f. eax/ecx/edx keep the quotient>>7/divisor/remainder.
// Every exit leaves the flags of its last flag writer.

static inline void set_bit1(uint8_t* item, gp4::Regs* r) {
    const uint8_t b = static_cast<uint8_t>(item[0x6f] | 2u);
    item[0x6f] = b;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(b));
}

// FUNCTION: GP4 0x0051e0bb
void __cdecl FUN_0051e0bb(gp4::Regs* r) {
    uint8_t* item = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007ab553);
    r->ebx = (r->ebx & 0xffffff00u) | mode;

    if (mode == 1u) {
        const uint32_t bit = GP4_GLOBAL(uint8_t, 0x00679d1b) & 2u;
        if (bit != 0u) {
            r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(bit));
            return;
        }
        set_bit1(item, r);
        return;
    }

    gp4::call_regs(0x0051ddda, *r);
    if ((r->eflags & gp4::flags::CF) == 0u) {
        if ((r->eflags & gp4::flags::ZF) == 0u) {
            set_bit1(item, r);
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

    const int32_t first = GP4_GLOBAL(int32_t, 0x007ab148);
    const int32_t second = GP4_GLOBAL(int32_t, 0x007ab14c);
    int32_t position = GP4_GLOBAL(int32_t, 0x007ab160);
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
    item[0x6c] = al;

    r->eax = result;
    r->ecx = range;
    r->edx = static_cast<uint32_t>(remainder);

    if (static_cast<int8_t>(al) < 0) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(al));
        return;
    }
    if (static_cast<int8_t>(al) < 0x40) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(al, 0x40u));
        return;
    }
    set_bit1(item, r);
}
GP4_IMPL(0x0051e0bb, FUN_0051e0bb, "regs(esi:ptr[0x78]:bytes, eax:u32, ebx:u32, ecx:u32, edx:u32) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=bits_006f2d90+axis_0051e0bb")
