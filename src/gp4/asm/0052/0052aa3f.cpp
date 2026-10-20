#include <gp4/gp4.h>
#include <gp4/flags.h>

// Draw handler of a menu list item (esi = the item record). Unless bit 4 of the item flag byte +0x1f is set:
// 0x54cc96 sets the draw colour (0x318 when bit 5 of +0x1f is set, else 0x7fff), 0x531af4 sets the fill colour
// (0x2108 when the item id +0xc equals the selected id byte [0x82fd45], else 0x1084), and 0x549db9 queues the
// rectangle (words +4/+6/+8/+0xa) into the sorted pool. The item's list cell (+0xe: dword 0 = first offset,
// dword 1 = entry count, bytes from +0xc) is then indexed by the dword +0x26: when offset + index is below the
// count, the entry byte is compared with [0x007c2f28] (equal -> 0x52aad1 draws the highlight), the rectangle
// corner x is stored through 0x54cbca (ebx = left + 2 + 0x14) and 0x54d1a2 draws the text-table entry
// 0xb90 + entry. pushal/popal restore every register; the flags are those the last instruction/call left.
// FUNCTION: GP4 0x0052aa3f
void __cdecl FUN_0052aa3f(gp4::Regs* r) {
    const uint8_t* item = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t itemFlags = item[0x1f];
    uint32_t arith = gp4::flags::logic8(itemFlags & 0x10u);
    if ((itemFlags & 0x10u) != 0u) {
        r->eflags = gp4::flags::merge(r->eflags, arith);
        return;
    }

    gp4::Regs c = *r;
    c.eax = 0x7fffu;
    arith = gp4::flags::logic8(itemFlags & 0x20u);
    if ((itemFlags & 0x20u) != 0u) {
        c.eax = 0x318u;
    }
    c.eflags = gp4::flags::merge(c.eflags, arith);
    gp4::call_regs(0x0054cc96, c);

    c.eax = 0x1084u;
    const uint32_t selected = GP4_GLOBAL(uint8_t, 0x0082fd45);
    c.edx = (c.edx & 0xffffff00u) | selected;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(selected, item[0x0c]));
    if (selected == item[0x0c]) {
        c.eax = 0x2108u;
    }
    gp4::call_regs(0x00531af4, c);

    c.ebx = *reinterpret_cast<const uint16_t*>(item + 0x04);
    c.eax = *reinterpret_cast<const uint16_t*>(item + 0x06);
    c.ebp = *reinterpret_cast<const uint16_t*>(item + 0x08);
    c.ecx = *reinterpret_cast<const uint16_t*>(item + 0x0a);
    gp4::call_regs(0x00549db9, c);
    gp4::cdecl_<0x00531b2e, void>();
    gp4::cdecl_<0x0054d70a, void>();

    arith = gp4::flags::add32(c.ebx, 2u);
    c.ebx += 2u;
    const uint8_t* cell = reinterpret_cast<const uint8_t*>(
        static_cast<uintptr_t>(*reinterpret_cast<const uint32_t*>(item + 0x0e)));
    const uint32_t offset = *reinterpret_cast<const uint32_t*>(cell);
    const uint32_t count = *reinterpret_cast<const uint32_t*>(cell + 4);
    uint32_t index = *reinterpret_cast<const uint32_t*>(item + 0x26);
    c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(cell));
    arith = gp4::flags::add32(index, offset);
    index += offset;
    arith = gp4::flags::sub32(index, count);
    c.edx = index;
    if (index >= count) {
        r->eflags = gp4::flags::merge(r->eflags, arith);
        return;
    }

    arith = gp4::flags::add32(index, 0x0cu);
    index += 0x0cu;
    c.edx = index;
    c.esi = cell[index];
    arith = gp4::flags::sub32(c.esi, GP4_GLOBAL(uint32_t, 0x007c2f28));
    if (c.esi == GP4_GLOBAL(uint32_t, 0x007c2f28)) {
        c.eflags = gp4::flags::merge(c.eflags, arith);
        gp4::call_regs(0x0052aad1, c);
        arith = c.eflags;
    }
    arith = gp4::flags::add32(c.ebx, 0x14u);
    c.ebx += 0x14u;
    c.eflags = gp4::flags::merge(c.eflags, arith);
    gp4::call_regs(0x0054cbca, c);
    arith = gp4::flags::add32(c.esi, 0xb90u);
    c.esi += 0xb90u;
    c.eflags = gp4::flags::merge(c.eflags, arith);
    gp4::call_regs(0x0054d1a2, c);

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0052aa3f, FUN_0052aa3f, "regs(ebx:u32, ecx:u32, edx:u32, esi:u32{=0x8501ff,0x850229,0x850253,0x85027d,0x8502a7,0x8502d1,0x8517e0,0x85180a,0x851834,0x85185e,0x851888,0x8518b2}) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9+script_0054d798+textout_0054d211+drv_0052aa3f")
