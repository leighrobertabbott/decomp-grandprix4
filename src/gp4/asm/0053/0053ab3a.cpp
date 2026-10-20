#include <gp4/gp4.h>
#include <gp4/flags.h>

// Queues the rectangle of the menu item esi and selects a style offset in edx. Unless bit 4 of the
// item's flag byte +0x1f is set, the item's unsigned words +4 (ebx), +6 (eax), +8 (ebp) and +0xa (ecx)
// are loaded and, inside a pushal/popal, the empty hook 0x0054a1d7 runs and the sorted-list insert
// 0x00549db9 is fed ebx, eax, ebp + 1 and ecx + 1. Then edx grows by 4 when bit 5 of +0x1f is set;
// otherwise by 2 when [0x007aafd3] == 0xff and [0x007ab1a0], [0x007aafe0] are not 0xff, and by 1 when
// the item's id byte +0xc equals the selection byte [0x0082fd45]. The empty routine 0x00531b35 ends
// it. eax/ebx/ebp/ecx come back as the four words; the flags are those of the last flag writer:
// `test +0x1f, 0x10` (early exit), the `add edx, 4`, the `add edx, 1`, or the `cmp` of the id byte with
// its carry forced by stc/clc.
// FUNCTION: GP4 0x0053ab3a
void __cdecl FUN_0053ab3a(gp4::Regs* r) {
    const uint32_t record = r->esi;
    const uint32_t hidden = GP4_FIELD(uint8_t, record, 0x1f) & 0x10u;
    uint32_t arith = gp4::flags::logic8(hidden);
    if (hidden != 0u) {
        r->eflags = gp4::flags::merge(r->eflags, arith);
        return;
    }

    const uint32_t ebx = GP4_FIELD(uint16_t, record, 4);
    const uint32_t eax = GP4_FIELD(uint16_t, record, 6);
    const uint32_t ebp = GP4_FIELD(uint16_t, record, 8);
    const uint32_t ecx = GP4_FIELD(uint16_t, record, 0xa);

    gp4::cdecl_<0x0054a1d7, void>();

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = eax;
    c.ebx = ebx;
    c.ebp = ebp + 1u;
    c.ecx = ecx + 1u;
    gp4::call_regs(0x00549db9, c);

    uint32_t edx = r->edx;
    const uint32_t disabled = GP4_FIELD(uint8_t, record, 0x1f) & 0x20u;
    arith = gp4::flags::logic8(disabled);
    if (disabled != 0u) {
        arith = gp4::flags::add32(edx, 4u);
        edx += 4u;
    } else {
        if (GP4_GLOBAL(uint8_t, 0x007aafd3) == 0xffu && GP4_GLOBAL(uint8_t, 0x007ab1a0) != 0xffu &&
            GP4_GLOBAL(uint8_t, 0x007aafe0) != 0xffu) {
            edx += 2u;
        }
        const uint32_t id = GP4_FIELD(uint8_t, record, 0x0c);
        const uint32_t selected = GP4_GLOBAL(uint8_t, 0x0082fd45);
        arith = gp4::flags::sub8(id, selected) & ~static_cast<uint32_t>(gp4::flags::CF);
        if (id == selected) {
            arith |= gp4::flags::CF;
            arith = gp4::flags::add32(edx, 1u);
            edx += 1u;
        }
    }
    gp4::cdecl_<0x00531b35, void>();

    r->eax = eax;
    r->ebx = ebx;
    r->ebp = ebp;
    r->ecx = ecx;
    r->edx = edx;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0053ab3a, FUN_0053ab3a, "regs(esi:ptr[0x40]:u8{0,0x10,0x20,0x30,0x80,0xff}, edx:u32) -> (eax, ebx, ebp, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9")
