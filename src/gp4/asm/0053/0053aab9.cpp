#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item "queue and style" routine, sibling of 0x0053ab3a with one more step: unless bit 4 of the
// item's flag byte +0x1f is set, the item's unsigned words +4 (ebx), +6 (eax), +8 (ebp) and +0xa (ecx)
// are loaded and, inside a pushal/popal, the empty hook 0x0054a1d7 runs and the sorted-list insert
// 0x00549db9 is fed ebx, eax, ebp + 1 and ecx + 1. Then edx grows by 4 when bit 5 of +0x1f is set;
// otherwise by 2 when [0x007aafd3] == 0xff and [0x007ab1a0], [0x007aafe0] are not 0xff, by 1 when the
// item's id byte +0xc equals the selection byte [0x0082fd45], and bit 7 of +0x1f moves the four words
// (ebx, eax, ebp, ecx) up by one. The empty routine 0x00531b35 ends it. The flags are those of the last
// flag writer: `test +0x1f, 0x10` (early exit), `add edx, 4`, `add ecx, 1`, or `test +0x1f, 0x80`.
// FUNCTION: GP4 0x0053aab9
void __cdecl FUN_0053aab9(gp4::Regs* r) {
    const uint32_t record = r->esi;
    const uint32_t bits = GP4_FIELD(uint8_t, record, 0x1f);
    uint32_t arith = gp4::flags::logic8(bits & 0x10u);
    if ((bits & 0x10u) != 0u) {
        r->eflags = gp4::flags::merge(r->eflags, arith);
        return;
    }

    uint32_t ebx = GP4_FIELD(uint16_t, record, 4);
    uint32_t eax = GP4_FIELD(uint16_t, record, 6);
    uint32_t ebp = GP4_FIELD(uint16_t, record, 8);
    uint32_t ecx = GP4_FIELD(uint16_t, record, 0xa);

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
    if ((GP4_FIELD(uint8_t, record, 0x1f) & 0x20u) != 0u) {
        arith = gp4::flags::add32(edx, 4u);
        edx += 4u;
    } else {
        if (GP4_GLOBAL(uint8_t, 0x007aafd3) == 0xffu && GP4_GLOBAL(uint8_t, 0x007ab1a0) != 0xffu &&
            GP4_GLOBAL(uint8_t, 0x007aafe0) != 0xffu) {
            edx += 2u;
        }
        if (GP4_FIELD(uint8_t, record, 0x0c) == GP4_GLOBAL(uint8_t, 0x0082fd45)) {
            edx += 1u;
        }
        const uint32_t high = GP4_FIELD(uint8_t, record, 0x1f) & 0x80u;
        arith = gp4::flags::logic8(high);
        if (high != 0u) {
            ebx += 1u;
            eax += 1u;
            ebp += 1u;
            arith = gp4::flags::add32(ecx, 1u);
            ecx += 1u;
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
GP4_IMPL(0x0053aab9, FUN_0053aab9, "regs(esi:ptr[0x40]:u8{0,0x10,0x20,0x30,0x80,0x90,0xa0,0xff}, edx:u32) -> (eax, ebx, ebp, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9+sel_0053aab9")
