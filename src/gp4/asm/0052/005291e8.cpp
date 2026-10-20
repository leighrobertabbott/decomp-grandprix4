#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu item value handler (esi = the item, edx = the command) for byte lists that must stay strictly
// increasing (the cells 0x0081d1f2..0x0081d1f7 and 0x0081d228). Commands 2 and 3 run the plain handler
// 0x005394b1, then take the cell ebx = [item + 0xe] and its byte al: a cell other than the list head
// 0x0081d1f2 is raised to the previous cell + 1 when it is not greater (the cell 0x0081d228 uses the byte
// 0x0081d1f7 as its predecessor), then a cell other than 0x0081d228 is lowered to the following cell - 1 when
// it is not below it (the cell 0x0081d1f7 uses the byte 0x0081d228 as its successor when that is non-zero).
// Commands 5 and 6 clear the dword 0x0082fd88; command 1 does nothing. The flags are those of the last
// compare / inc / dec on the path.
static inline uint32_t inc8_keep_cf(uint32_t v, uint32_t in_flags) {
    return (gp4::flags::add8(v, 1u) & ~gp4::flags::CF) | (in_flags & gp4::flags::CF);
}
static inline uint32_t dec8_keep_cf(uint32_t v, uint32_t in_flags) {
    return (gp4::flags::sub8(v, 1u) & ~gp4::flags::CF) | (in_flags & gp4::flags::CF);
}
static inline bool signed_greater(uint32_t arith) {
    return (arith & gp4::flags::ZF) == 0u && (((arith & gp4::flags::SF) != 0u) == ((arith & gp4::flags::OF) != 0u));
}

// FUNCTION: GP4 0x005291e8
void __cdecl FUN_005291e8(gp4::Regs* r) {
    const uint32_t command = r->edx;

    if (command == 1u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(command, 1u));
        return;
    }
    if (command != 2u && command != 3u) {
        if (command == 5u) {
            GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
            r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(command, 5u));
            return;
        }
        if (command == 6u) {
            GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
        }
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(command, 6u));
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    gp4::call_regs(0x005394b1, c);

    const uint32_t item = r->esi;
    const uint32_t cell = GP4_FIELD(uint32_t, item, 0xe);
    uint32_t al = GP4_FIELD(uint8_t, cell, 0);
    uint32_t arith = c.eflags;

    arith = gp4::flags::sub32(cell, 0x0081d1f2u);
    if (cell != 0x0081d1f2u) {
        arith = gp4::flags::sub32(cell, 0x0081d228u);
        if (cell == 0x0081d228u) {
            const uint32_t head = GP4_GLOBAL(uint8_t, 0x0081d1f7);
            arith = gp4::flags::sub8(al, head);
            if (!signed_greater(arith)) {
                al = GP4_GLOBAL(uint8_t, 0x0081d1f7);
                arith = inc8_keep_cf(al, arith);
                al = (al + 1u) & 0xffu;
                GP4_GLOBAL(uint8_t, 0x0081d228) = static_cast<uint8_t>(al);
            }
        } else {
            const uint32_t prev = GP4_FIELD(uint8_t, cell, -1);
            arith = gp4::flags::sub8(al, prev);
            if (!signed_greater(arith)) {
                al = GP4_FIELD(uint8_t, cell, -1);
                arith = inc8_keep_cf(al, arith);
                al = (al + 1u) & 0xffu;
                GP4_FIELD(uint8_t, cell, 0) = static_cast<uint8_t>(al);
            }
        }
    }

    arith = gp4::flags::sub32(cell, 0x0081d228u);
    if (cell != 0x0081d228u) {
        arith = gp4::flags::sub32(cell, 0x0081d1f7u);
        if (cell == 0x0081d1f7u) {
            const uint32_t successor = GP4_GLOBAL(uint8_t, 0x0081d228);
            arith = gp4::flags::sub8(successor, 0u);
            if (successor != 0u) {
                const uint32_t limit = GP4_GLOBAL(uint8_t, 0x0081d228);
                arith = gp4::flags::sub8(al, limit);
                if (!(al < limit)) {
                    al = GP4_GLOBAL(uint8_t, 0x0081d228);
                    arith = dec8_keep_cf(al, arith);
                    al = (al - 1u) & 0xffu;
                    GP4_GLOBAL(uint8_t, 0x0081d1f7) = static_cast<uint8_t>(al);
                }
            }
        } else {
            const uint32_t next = GP4_FIELD(uint8_t, cell, 1);
            arith = gp4::flags::sub8(al, next);
            if (!(al < next)) {
                al = GP4_FIELD(uint8_t, cell, 1);
                arith = dec8_keep_cf(al, arith);
                al = (al - 1u) & 0xffu;
                GP4_FIELD(uint8_t, cell, 0) = static_cast<uint8_t>(al);
            }
        }
    }

    r->eax = (c.eax & 0xffffff00u) | al;
    r->ebx = cell;
    r->edx = c.edx;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005291e8, FUN_005291e8, "regs(eax:u32, ebx:u32, edx:u32{1,2,3,5,6,0,4,2,3,2,3}, esi:u32{=0x84f79c,0x84f822,0x84f8a8,0x84f92e,0x84f9b4,0x84fa3a,0x84fb1f}) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cells_005291e8")
