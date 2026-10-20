#include <gp4/gp4.h>
#include <gp4/flags.h>

#pragma pack(push, 1)
struct ScrollAxis {
    uint32_t Position;   // +0
    uint32_t Limit;      // +4
    uint32_t Size;       // +8: visible window
};
#pragma pack(pop)

// Doubles the visible window of one axis (capped at the limit, unsigned), moves the position by
// half the growth so the window stays centred, and returns limit * 0x4000 / size (signed, 0 for
// size 0). ecx keeps the new position, edx the high dword / remainder of the division.
static inline uint32_t ZoomOutAxis(gp4::Regs* r, ScrollAxis* axis) {
    uint32_t position = axis->Position + (axis->Size >> 1);
    uint32_t size = axis->Size << 1;
    if (size > axis->Limit) {
        size = axis->Limit;
    }
    axis->Size = size;
    position -= size >> 1;
    axis->Position = position;
    r->ecx = position;
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(axis->Limit)) * 0x4000;
    if (axis->Size == 0u) {
        r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
        return 0u;
    }
    int32_t remainder = 0;
    const int32_t zoom = gp4::idiv64(product, static_cast<int32_t>(axis->Size), &remainder);
    r->edx = static_cast<uint32_t>(remainder);
    return static_cast<uint32_t>(zoom);
}

// Scroll/zoom menu item handler selected by edx.
//  edx = 1: clears bit 7 of byte +0x1f of the esi item and, unless bit 7 of +0x1e is set, compares
//           byte +0xc - 1 (eax) with [ebx] through 0x005398c4 and sets bit 7 of +0x1f again when
//           equal; edx = 0 on return.
//  edx = 5: zooms both view axes out - block 0x8310c0 (zoom 0x83137b) then 0x8310cc (zoom
//           0x83137f) - each clamped by 0x00543bc3; edx is saved and restored, ebx ends at
//           0x8310cc, ecx at its new position and eax at its zoom divisor.
//  other:   nothing (flags of `cmp edx, 5`).
// FUNCTION: GP4 0x00543a93
void __cdecl FUN_00543a93(gp4::Regs* r) {
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
            if ((r->eflags & gp4::ZF) != 0u) {
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

    ScrollAxis* xAxis = GP4_ARRAY(ScrollAxis, 0x008310c0);
    r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(xAxis));
    r->eax = ZoomOutAxis(r, xAxis);
    GP4_GLOBAL(uint32_t, 0x0083137b) = r->eax;
    gp4::call_regs(0x00543bc3, *r);

    ScrollAxis* yAxis = GP4_ARRAY(ScrollAxis, 0x008310cc);
    r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(yAxis));
    r->eax = ZoomOutAxis(r, yAxis);
    GP4_GLOBAL(uint32_t, 0x0083137f) = r->eax;
    gp4::call_regs(0x00543bc3, *r);

    r->edx = command;
}
GP4_IMPL(0x00543a93, FUN_00543a93, "regs(edx:u32{1,5,0,2,5,5}, ebx:ptr[8]:u8{0,1,0xff}, esi:ptr[0x40]:u8{0,1,2,0x80,0x81}) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=scroll_00543340+zoom_00543a93")
