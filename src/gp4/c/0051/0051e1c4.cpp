#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sets byte +0x6d of the esi control record from the mode byte 0x7ab556 (also left in bl). Mode
// 0x1c: 0x7f when bit 4 of +0x6f is set, else 0. Modes 6..9 and 0x20..0x27: the position
// 0x7ab164 is clamped between the two end stops 0x7ab154 and 0x7ab150 (in either order), mapped to
// (pos - [0x7ab154]) * 0x4000 / ([0x7ab150] - [0x7ab154]) (a zero range divides by 1), clamped by
// magnitude to 0x3f80 and stored >> 7 (sar) - eax, ecx (divisor) and edx (remainder) keep the
// working values. Any other mode: nothing (flags of `cmp bl, 0x27`).
// FUNCTION: GP4 0x0051e1c4
void __cdecl FUN_0051e1c4(gp4::Regs* r) {
    uint8_t* item = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007ab556);
    r->ebx = (r->ebx & 0xffffff00u) | mode;

    if (mode == 0x1cu) {
        const uint32_t bit = item[0x6f] & 0x10u;
        item[0x6d] = static_cast<uint8_t>(bit != 0u ? 0x7fu : 0u);
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(bit));
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

    const int32_t first = GP4_GLOBAL(int32_t, 0x007ab150);
    const int32_t second = GP4_GLOBAL(int32_t, 0x007ab154);
    int32_t position = GP4_GLOBAL(int32_t, 0x007ab164);
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
    item[0x6d] = static_cast<uint8_t>(result);

    r->eax = result;
    r->ecx = range;
    r->edx = static_cast<uint32_t>(remainder);
    const uint32_t arith = gp4::flags::logic32(result) | ((level >> 6) & 1u);
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0051e1c4, FUN_0051e1c4, "regs(ebx:u32, esi:ptr[0x78]:bytes) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=axis_0051e1c4")
