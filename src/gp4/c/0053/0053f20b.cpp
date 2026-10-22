#include <gp4/gp4.h>
#include <gp4/flags.h>

// Points the third and fourth items of the front-end menu list rooted at 0x8484b0 at two text
// buffers: the +0xe text pointer of the third item becomes 0x7ab1a4 + 0x40 * [0x7ab4a4] and that of
// the fourth item the same address + 0x20 (the item links are followed from the root cell with
// `mov esi,[esi]`). Then the menu is run (esi = the root cell) with 0x53c8af, whose result selects
// one of two entries of the `jmp [eax]` table at 0x53f25c (result >= 2 counts as 1); both entries lead
// to the `ret` at 0x53f264. eax thus ends as 0x53f25c + 4 * min(result, 1) and the flags are those
// of the `add eax, 0x53f25c`.
// FUNCTION: GP4 0x0053f20b
void __cdecl FUN_0053f20b(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    const uint32_t text = static_cast<uint32_t>(GP4_GLOBAL(uint8_t, 0x007ab4a4)) * 0x40u + 0x007ab1a4u;
    uint32_t node = 0x008484b0u;
    for (uint32_t n = 3; n != 0; --n) {
        node = GP4_FIELD(uint32_t, node, 0);
    }
    GP4_FIELD(uint32_t, node, 0xe) = text;
    node = GP4_FIELD(uint32_t, node, 0);
    GP4_FIELD(uint32_t, node, 0xe) = text + 0x20u;

    c.eax = text;
    c.ebx = text + 0x20u;
    c.ecx = 0;
    c.esi = 0x008484b0u;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(text, 0x20u));
    gp4::call_regs(0x0053c8af, c);

    uint32_t index = c.eax;
    if (index >= 2u) {
        index = 1u;
    }
    index <<= 2;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(index, 0x0053f25cu));
    c.eax = index + 0x0053f25cu;
    *r = c;
}
GP4_IMPL(0x0053f20b, FUN_0053f20b, "regs(edx:u32, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=case_0054fd4d+anim_0052822a+slots_005388ed+cells_00527eae+slot8_005388ba+surfcopy_00549d55+frame_0082fd70; gfields={0x008484b0:ptr[0x40]:{*bytes, 0:ptr[0x40]:{*bytes, 0:ptr[0x40]:{*bytes, 0:ptr[0x40]:bytes}}}}; emu=skip:win32 PeekMessageA")
