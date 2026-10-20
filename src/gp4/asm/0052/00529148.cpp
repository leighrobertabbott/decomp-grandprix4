#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item value handler for commands 2 and 3 (esi = the item, ebx = the cell inside the byte lists at
// 0x0081d1f2 / 0x0081d228, the minimum is the dword +0x26 of the item): reads the cell (0x00539944) and,
// when value - 1 is at least the minimum, stores value - 1 back (0x00539a13) unless it is not greater
// than the entry before the cell (the signed byte compare; the first list head 0x0081d1f2 always
// stores, and the cell 0x0081d228 compares with the byte [0x0081d1f7] when [0x0081d228] is non-zero).
// eax comes back as value - 1 on those paths; the flags are those of the last writer on the path.
// FUNCTION: GP4 0x00529148
void __cdecl FUN_00529148(gp4::Regs* r) {
    const uint32_t command = r->edx;
    const uint32_t item = r->esi;
    uint32_t arith;

    if (command == 1u) {
        arith = gp4::flags::sub32(command, 1u);
    } else if (command != 2u && command != 3u) {
        arith = gp4::flags::sub32(command, 3u);
    } else {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.st_xin_mask = 0;
        c.st_xout_mask = 0;
        gp4::call_regs(0x00539944, c);
        const uint32_t value = c.eax;
        const uint32_t eax = value - 1u;
        const uint32_t cell = r->ebx;
        r->eax = eax;
        arith = gp4::flags::sub32(value, 1u);
        if (value != 0u) {
            const uint32_t minimum = GP4_FIELD(uint32_t, item, 0x26);
            arith = gp4::flags::sub32(eax, minimum);
            if (eax >= minimum) {
                bool store = false;
                bool compare = false;
                bool useHead = false;
                arith = gp4::flags::sub32(cell, 0x0081d1f2u);
                if (cell == 0x0081d1f2u) {
                    store = true;
                } else {
                    arith = gp4::flags::sub8(GP4_GLOBAL(uint8_t, 0x0081d228), 0u);
                    if (GP4_GLOBAL(uint8_t, 0x0081d228) == 0u) {
                        compare = true;
                    } else {
                        arith = gp4::flags::sub32(cell, 0x0081d228u);
                        if (cell == 0x0081d228u) {
                            useHead = true;
                        } else {
                            compare = true;
                        }
                    }
                }
                if (compare || useHead) {
                    const uint8_t other = useHead ? GP4_GLOBAL(uint8_t, 0x0081d1f7)
                                                  : GP4_FIELD(uint8_t, cell, -1);
                    arith = gp4::flags::sub8(eax, other);
                    const bool le = (arith & gp4::flags::ZF) != 0u
                        || (((arith & gp4::flags::SF) != 0u) != ((arith & gp4::flags::OF) != 0u));
                    store = !le;
                }
                if (store) {
                    gp4::Regs s = *r;
                    s.st_in = 0;
                    s.st_out = 0;
                    s.st_xin_mask = 0;
                    s.st_xout_mask = 0;
                    s.eax = eax;
                    gp4::call_regs(0x00539a13, s);
                    arith = s.eflags;
                }
            }
        }
    }

    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00529148, FUN_00529148, "regs(eax:u32, edx:u32{1,2,3,2,3,0,5}, ebx:u32{=0x81d1f2,0x81d1f3,0x81d1fc,0x81d1fd,0x81d1fe,0x81d1ff,0x81d224,0x81d225,0x81d226,0x81d227,0x81d228,0x81d228,0x81d228,0x81d228}, esi:ptr[0x40]:layout=item_00529148) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=lists_005295e0+vals_00529148")
