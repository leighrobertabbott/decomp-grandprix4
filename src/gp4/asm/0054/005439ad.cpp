#include <gp4/gp4.h>
#include <gp4/flags.h>

#pragma pack(push, 1)
struct PlotAxis {
    uint32_t Origin;   // +0: lower corner
    uint32_t Range;    // +4
    uint32_t Span;     // +8: visible width
};
#pragma pack(pop)

// Fits one plot axis to the corners `first`/`second` (unsigned min/max): origin = min, span =
// max - min but at least range >> shift; returns range * 0x4000 / span (signed, 0 for span 0).
// ecx keeps the minimum span, edx the high dword / remainder of the division.
static inline uint32_t FitAxis(gp4::Regs* r, PlotAxis* axis, uint32_t first, uint32_t second,
                               unsigned shift) {
    uint32_t low = first;
    uint32_t high = second;
    if (high < low) {
        low = second;
        high = first;
    }
    axis->Origin = low;
    uint32_t span = high - low;
    const uint32_t minimum = axis->Range >> shift;
    if (span < minimum) {
        span = minimum;
    }
    axis->Span = span;
    r->ecx = minimum;
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(axis->Range)) * 0x4000;
    if (axis->Span == 0u) {
        r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
        return 0u;
    }
    int32_t remainder = 0;
    const int32_t zoom = gp4::idiv64(product, static_cast<int32_t>(axis->Span), &remainder);
    r->edx = static_cast<uint32_t>(remainder);
    return static_cast<uint32_t>(zoom);
}

// edx = 1: clears bit 7 of byte +0x1f of the esi item and, unless bit 7 of +0x1e is set, compares
// byte +0xc - 1 (eax) with [ebx] through 0x005398c4 and sets bit 7 of +0x1f again when equal;
// edx = 0 on return. edx = 5 (when the mode byte 0x82fcf0 is nonzero): fits the x axis block
// 0x8310c0 to the corners 0x82fcd0/0x82fcd8 (zoom 0x83137b) and, unless 0x8310b5 is set, the y
// axis block 0x8310cc to 0x82fcd4/0x82fcdc (zoom 0x83137f), each clamped by 0x00543bc3; edx is
// saved and restored. Other edx: nothing (flags of `cmp edx, 5`).
// FUNCTION: GP4 0x005439ad
void __cdecl FUN_005439ad(gp4::Regs* r) {
    const uint32_t command = r->edx;
    if (command == 1u) {
        uint8_t* item = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
        item[0x1f] = static_cast<uint8_t>(item[0x1f] & 0x7fu);
        const uint32_t locked = item[0x1e] & 0x80u;
        if (locked != 0u) {
            r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(locked));
        } else {
            r->eax = static_cast<uint32_t>(item[0xc]) - 1u;
            gp4::call_regs(0x005398c4, *r);
            if ((r->eflags & 0x40u) != 0u) {
                item[0x1f] = static_cast<uint8_t>(item[0x1f] | 0x80u);
                r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(item[0x1f]));
            }
        }
        r->edx = 0u;
        return;
    }
    if (command != 5u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(command, 5u));
        return;
    }

    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x0082fcf0);
    if (mode == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(mode, 0u));
        return;
    }

    PlotAxis* xAxis = GP4_ARRAY(PlotAxis, 0x008310c0);
    r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(xAxis));
    r->eax = FitAxis(r, xAxis, GP4_GLOBAL(uint32_t, 0x0082fcd0), GP4_GLOBAL(uint32_t, 0x0082fcd8), 6u);
    GP4_GLOBAL(uint32_t, 0x0083137b) = r->eax;
    gp4::call_regs(0x00543bc3, *r);

    const uint8_t xOnly = GP4_GLOBAL(uint8_t, 0x008310b5);
    if (xOnly != 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(xOnly, 0u));
    } else {
        PlotAxis* yAxis = GP4_ARRAY(PlotAxis, 0x008310cc);
        r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(yAxis));
        r->eax = FitAxis(r, yAxis, GP4_GLOBAL(uint32_t, 0x0082fcd4), GP4_GLOBAL(uint32_t, 0x0082fcdc), 4u);
        GP4_GLOBAL(uint32_t, 0x0083137f) = r->eax;
        gp4::call_regs(0x00543bc3, *r);
    }
    r->edx = command;
}
GP4_IMPL(0x005439ad, FUN_005439ad, "regs(edx:u32{1,5,0,2,5,5}, ebx:ptr[8]:u8{0,1,0xff}, esi:ptr[0x40]:u8{0,1,2,0x80,0x81}) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=scroll_00543340+corners_005439ad")
