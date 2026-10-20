#include <gp4/gp4.h>
#include <gp4/flags.h>

// Draws the 22 text rows of a key-binding list. For each row index [0x00822280] = 0..21: the text
// colour is set (0x00546672) to the slot value of 0x00549c8a (key = the row index) when the byte
// [0x00831d65] is 0xff, else to 0; the cursor is placed at (0x13, row + 7) by 0x00546293; and when the
// row's code byte 0x00831d67[row] is nonzero and either [0x00831a97] is clear or the code's entry in the
// flag table 0x007c2a47 has bit 0x40 or 0x80, the text 0x0b8f + code is drawn by 0x00546173.
// Register results (verified against the disassembly): `movzx eax, byte [eax+0x831d67]` leaves eax =
// code; a drawing row then does `add eax, 0xb8f; mov esi, eax` so eax = esi = 0x0b8f + code enter
// 0x00546173 and come back as that callee left them; a row that draws nothing leaves eax = code.
// The final flags are those of `cmp [0x00822280], 0x16` (equal).
// FUNCTION: GP4 0x005499eb
void __cdecl FUN_005499eb(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    GP4_GLOBAL(uint32_t, 0x00822280) = 0u;
    do {
        const uint32_t row = GP4_GLOBAL(uint32_t, 0x00822280);
        c.eax = row;
        uint32_t colour = 0u;
        if (GP4_GLOBAL(uint8_t, 0x00831d65) == 0xffu) {
            gp4::call_regs(0x00549c8a, c);
            colour = c.eax;
        } else {
            c.eax = 0u;
        }
        const uint32_t colourArgs[1] = {colour};
        gp4::call_regs_stdcall(0x00546672, c, colourArgs);

        const uint32_t cursorArgs[2] = {0x13u, GP4_GLOBAL(uint32_t, 0x00822280) + 7u};
        gp4::call_regs_stdcall(0x00546293, c, cursorArgs);

        const uint32_t code = GP4_ARRAY(const uint8_t, 0x00831d67)[GP4_GLOBAL(uint32_t, 0x00822280)];
        c.eax = code;
        if (GP4_GLOBAL(uint8_t, 0x00831a97) == 0u || (GP4_ARRAY(const uint8_t, 0x007c2a47)[code] & 0xc0u) != 0u) {
            if (code != 0u) {
                c.eax = code + 0xb8fu;
                c.esi = c.eax;
                gp4::call_regs(0x00546173, c);
            }
        }
        GP4_GLOBAL(uint32_t, 0x00822280) = GP4_GLOBAL(uint32_t, 0x00822280) + 1u;
    } while (GP4_GLOBAL(uint32_t, 0x00822280) != 0x16u);

    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(0x16u, 0x16u));
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x005499eb, FUN_005499eb, "regs(eax:u32, esi:u32) -> (eax, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keys_00831d67+cur_00549849+tabs_00831d1c+cursor_00545f09+surf_0054654a+script_00545efe+txt_005499eb")
