#include <gp4/gp4.h>
#include <gp4/flags.h>

// Picks the pointer from the 0x0081d11c table that belongs to the current menu cursor cell [0x00823690].
//  - session type [0x007ad898] == 5: the cell is doubled (`shl eax, 1`), its low byte replaced by the byte
//    0x007c2b24[doubled] (the upper bits of the doubled cell stay) and the table is indexed by that value.
//  - [0x0082fd34] is the list head 0x0083e782 or 0x0083ef8c: table[cell].
//  - byte [0x007aafd3] != 0, or 0x005419d7 leaves ZF: the fixed default 0x0081cedc.
//  - otherwise table[cell] unless one of the driver bytes 0x007c2a48 + 2 * cell (+1) has its top two bits
//    equal to 0x80; then the first driver byte (22 of them) with bit 7 set and bit 6 clear selects table[its
//    index >> 1], and 0x0081ceac when there is none.
// dl and ebx are only written on the last path (dl = the masked driver byte, ebx = the cell).
// FUNCTION: GP4 0x00528535
void __cdecl FUN_00528535(gp4::Regs* r) {
    const uint32_t* const table = GP4_ARRAY(const uint32_t, 0x0081d11c);

    if (GP4_GLOBAL(uint32_t, 0x007ad898) == 5u) {
        const uint32_t cell = GP4_GLOBAL(uint32_t, 0x00823690);
        const uint32_t doubled = cell << 1;
        // shl eax, 1: CF = old bit 31, OF = CF ^ result bit 31, AF = 0.
        const uint32_t carry = cell >> 31;
        const uint32_t shl_flags = carry | ((carry ^ (doubled >> 31)) != 0u ? gp4::flags::OF : 0u) | gp4::flags::logic32(doubled);
        const uint32_t selector = (doubled & 0xffffff00u) | GP4_ARRAY(const uint8_t, 0x007c2b24)[doubled];
        r->eax = table[selector];
        r->eflags = gp4::flags::merge(r->eflags, shl_flags);
        return;
    }

    const uint32_t head = GP4_GLOBAL(uint32_t, 0x0082fd34);
    uint32_t arith = gp4::flags::sub32(head, 0x0083e782u);
    if (head != 0x0083e782u) {
        arith = gp4::flags::sub32(head, 0x0083ef8cu);
    }
    if (head == 0x0083e782u || head == 0x0083ef8cu) {
        r->eax = table[GP4_GLOBAL(uint32_t, 0x00823690)];
        r->eflags = gp4::flags::merge(r->eflags, arith);
        return;
    }

    const uint32_t mode = GP4_GLOBAL(uint8_t, 0x007aafd3);
    if (mode != 0u) {
        r->eax = 0x0081cedcu;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(mode, 0u));
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    gp4::call_regs(0x005419d7, c);
    if ((c.eflags & gp4::flags::ZF) != 0u) {
        r->eax = 0x0081cedcu;
        r->eflags = gp4::flags::merge(r->eflags, c.eflags);
        return;
    }

    const uint32_t cell = GP4_GLOBAL(uint32_t, 0x00823690);
    const uint8_t* const drivers = GP4_ARRAY(const uint8_t, 0x007c2a48);
    r->ebx = cell;
    uint32_t pointer = table[cell];

    uint32_t top = static_cast<uint32_t>(drivers[cell * 2u]) & 0xc0u;
    r->edx = (r->edx & 0xffffff00u) | top;
    arith = gp4::flags::sub8(top, 0x80u);
    if (top != 0x80u) {
        top = static_cast<uint32_t>(drivers[cell * 2u + 1u]) & 0xc0u;
        r->edx = (r->edx & 0xffffff00u) | top;
        arith = gp4::flags::sub8(top, 0x80u);
        if (top != 0x80u) {
            uint32_t slot = 0u;
            for (;;) {
                const uint32_t b = drivers[slot];
                if ((b & 0x40u) == 0u && (b & 0x80u) != 0u) {
                    // sub eax, 0x7c2a48 ; shr eax, 1
                    const uint32_t half = slot >> 1;
                    arith = (slot & 1u) | gp4::flags::logic32(half);
                    pointer = table[half];
                    break;
                }
                ++slot;
                arith = gp4::flags::sub32(0x007c2a48u + slot, 0x007c2a5eu);
                if (slot == 0x16u) {
                    pointer = 0x0081ceacu;
                    break;
                }
            }
        }
    }
    r->eax = pointer;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00528535, FUN_00528535, "regs(edx:u32) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=text_005325bc+mode_007aafd3+flag_007ab020+cell_0053ae38+drv22_0052d402")
