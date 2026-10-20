#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item value handler selected by the command in edx (esi = the item, ebx = its value cell, the mask is the
// dword +0x26 of the item), the variant of 0x00539031 that also records the item type and refreshes the pages
// (twin of 0x005287f7 with the mask test inverted):
//   1     clear bit 7 of the item flags (+0x1f); unless the type byte +0x0c is 0x0c or more (signed) while
//         [0x0081d1d6] is nonzero, set bit 7 again when the mask and the cell share no bit;
//   2, 3  run 0x00539031 (command 2 clears the mask bits in the cell), map the item type to the mode byte
//         (0x005287d3), then refresh the pages (0x00528927, 0x00529408, 0x0052a545, 0x005413e9);
//   5, 6  clear the dword [0x0082fd88].
// edx comes back 0 for every command; the flags are those of the last writer on the path.
// FUNCTION: GP4 0x00528863
void __cdecl FUN_00528863(gp4::Regs* r) {
    const uint32_t command = r->edx;
    const uint32_t item = r->esi;

    if (command == 1u) {
        GP4_FIELD(uint8_t, item, 0x1f) = static_cast<uint8_t>(GP4_FIELD(uint8_t, item, 0x1f) & 0x7fu);
        const uint32_t type = GP4_FIELD(uint8_t, item, 0x0c);
        uint32_t arith = gp4::flags::sub8(type, 0x0cu);
        const bool skip = static_cast<int8_t>(type) >= 0x0c;
        if (skip) {
            const uint32_t selector = GP4_GLOBAL(uint8_t, 0x0081d1d6);
            arith = gp4::flags::sub8(selector, 0u);
            if (selector != 0u) {
                r->eax = (r->eax & 0xffffff00u) | type;
                r->edx = 0u;
                r->eflags = gp4::flags::merge(r->eflags, arith);
                return;
            }
        }
        const uint32_t cell = GP4_FIELD(uint32_t, r->ebx, 0);
        const uint32_t value = GP4_FIELD(uint32_t, item, 0x26) & cell;
        r->eax = value;
        arith = gp4::flags::logic32(value);
        if (value == 0u) {
            const uint8_t marked = static_cast<uint8_t>(GP4_FIELD(uint8_t, item, 0x1f) | 0x80u);
            GP4_FIELD(uint8_t, item, 0x1f) = marked;
            arith = gp4::flags::logic8(marked);
        }
        r->edx = 0u;
        r->eflags = gp4::flags::merge(r->eflags, arith);
        return;
    }

    if (command == 2u || command == 3u) {
        gp4::Regs c = *r;
        gp4::call_regs(0x00539031, c);
        gp4::call_regs(0x005287d3, c);
        gp4::call_regs(0x00528927, c);
        gp4::call_regs(0x00529408, c);
        gp4::call_regs(0x0052a545, c);
        gp4::call_regs(0x005413e9, c);
        c.edx = 0u;
        *r = c;
        return;
    }

    uint32_t arith;
    if (command == 5u) {
        arith = gp4::flags::sub32(command, 5u);
        GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
    } else {
        arith = gp4::flags::sub32(command, 6u);
        if (command == 6u) {
            GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
        }
    }
    r->edx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00528863, FUN_00528863, "regs(eax:u32, edx:u32{1,2,3,2,3,0,5,6}, ebx:ptr[8]:layout=cell_005292c0, esi:ptr[0x40]:layout=item_00529148) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=session_004fd1a0+cobj_0084e9bb+trk_007ad894+ramp_0081d1f2+menu_00529408")
