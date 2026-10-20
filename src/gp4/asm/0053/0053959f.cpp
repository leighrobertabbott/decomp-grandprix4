#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu slider value handler selected by the command in edx (esi = the item), sibling of 0x0053952b.
// Commands 2 and 3 (and 4 when bit 2 of the item state byte +0x1e is set) clear the flag 0x007ab003
// (0x0053daa6) and map the pointer row [0x82fd98] onto the item's value range: with top = word +6 and
// bottom = word +0xa, offset = max([0x82fd98] - top, 0) and rows = bottom - top + 1, the value is
// ((dword +0x2e - dword +0x2a) * offset + rows / 2) / rows + dword +0x2a (the quotient saturates to rows
// when the 64-bit dividend would overflow), clamped to dwords +0x22 (low) and +0x26 (high), and stored
// through the cell pointer +0xe by 0x00539a13 (ebx = the pointer). No rows (bottom < top) runs the empty
// routine 0x0054e232 inside pushal/popal instead. Commands 5 and 6 clear the dword [0x0082fd88]. edx comes
// back 0; the flags are those of the last writer on the path taken.
// FUNCTION: GP4 0x0053959f
void __cdecl FUN_0053959f(gp4::Regs* r) {
    const uint32_t command = r->edx;
    const uint32_t item = r->esi;
    uint32_t arith;
    bool update = false;

    if (command == 1u) {
        arith = gp4::flags::sub32(command, 1u);
    } else if (command == 2u) {
        arith = gp4::flags::sub32(command, 2u);
        update = true;
    } else if (command == 3u) {
        arith = gp4::flags::sub32(command, 3u);
        update = true;
    } else if (command == 4u) {
        const uint8_t state = GP4_FIELD(uint8_t, item, 0x1e);
        arith = gp4::flags::logic8(state & 4u);
        update = (state & 4u) != 0u;
    } else if (command == 5u) {
        arith = gp4::flags::sub32(command, 5u);
        GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
    } else {
        arith = gp4::flags::sub32(command, 6u);
        if (command == 6u) {
            GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
        }
    }

    if (update) {
        gp4::cdecl_<0x0053daa6, void>();
        const uint32_t top = GP4_FIELD(uint16_t, item, 6);
        const uint32_t bottom = GP4_FIELD(uint16_t, item, 0xa);
        const int32_t pointer_row = GP4_GLOBAL(int32_t, 0x0082fd98);
        uint32_t offset = static_cast<uint32_t>(pointer_row) - top;
        if (pointer_row < static_cast<int32_t>(top)) {
            offset = 0u;
        }
        const uint32_t span = GP4_FIELD(uint32_t, item, 0x2e) - GP4_FIELD(uint32_t, item, 0x2a);
        const uint64_t product = static_cast<uint64_t>(span) * offset;
        const uint32_t rows_less_one = bottom - top;
        const int32_t rows = static_cast<int32_t>(rows_less_one) + 1;

        if (rows <= 0) {
            gp4::cdecl_<0x0054e232, void>();
            arith = gp4::flags::add32(rows_less_one, 1u);
            r->eax = static_cast<uint32_t>(product);
            r->ebx = top;
            r->ecx = rows_less_one + 1u;
            r->ebp = bottom;
        } else {
            const uint32_t count = static_cast<uint32_t>(rows);
            const uint64_t dividend = product + (count >> 1);
            uint32_t value;
            if (count <= static_cast<uint32_t>(dividend >> 32)) {
                value = count;
            } else {
                uint32_t remainder;
                value = gp4::div64(dividend, count, &remainder);
            }
            value += GP4_FIELD(uint32_t, item, 0x2a);
            if (static_cast<int32_t>(value) < GP4_FIELD(int32_t, item, 0x22)) {
                value = GP4_FIELD(uint32_t, item, 0x22);
            }
            if (static_cast<int32_t>(value) > GP4_FIELD(int32_t, item, 0x26)) {
                value = GP4_FIELD(uint32_t, item, 0x26);
            }
            gp4::Regs c = *r;
            c.st_in = 0;
            c.st_out = 0;
            c.st_xin_mask = 0;
            c.st_xout_mask = 0;
            c.eax = value;
            c.ebx = GP4_FIELD(uint32_t, item, 0xe);
            gp4::call_regs(0x00539a13, c);
            r->eax = c.eax;
            r->ebx = c.ebx;
            r->ecx = count;
            r->ebp = bottom;
            arith = c.eflags;
        }
    }

    r->edx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0053959f, FUN_0053959f, "regs(edx:u32{1,2,3,4,5,6,0,4,2,3}, esi:ptr[0x40]:layout=rec_00529ee2) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
