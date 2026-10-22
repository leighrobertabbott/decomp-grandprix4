#include <gp4/gp4.h>
#include <gp4/flags.h>

// Flags of `imul r32, r32, imm` (oracle model): CF = OF = the product does not fit 32 bits,
// AF = 0, PF/ZF/SF from the low dword.
static inline uint32_t imulFlags32(int32_t value, int32_t factor) {
    const int64_t product = static_cast<int64_t>(value) * factor;
    const uint32_t low = static_cast<uint32_t>(product);
    const bool overflow = product != static_cast<int64_t>(static_cast<int32_t>(low));
    return (overflow ? (gp4::flags::CF | gp4::flags::OF) : 0u) | gp4::flags::szp(low, 32);
}

// Twin of 0x0053f20b for the menu list rooted at 0x8485a6: the +0xe text pointer of its third item
// becomes 0x7ab1a4 + 0x40 * [0x7ab4a4] and that of the fourth item the same address + 0x20; the menu is
// run (esi = the root cell) with 0x53c8af, whose result (>= 2 counts as 1) selects one of two entries of
// the `jmp [eax]` table at 0x53f2b8. Entry 1 is the `ret`; entry 0 (the result 0) deletes the text slot
// of index [0x7ab4a4]: the following slots (up to 12 slots of 0x40 bytes) move down by one slot
// (`rep movsb`, ecx = (11 - index) * 0x40) and the two bytes after the moved block, at +0 and +0x20,
// are cleared. eax ends as the slot address (case 0) or 0x53f2b8 + 4 (case 1).
// FUNCTION: GP4 0x0053f265
void __cdecl FUN_0053f265(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    const uint32_t text = static_cast<uint32_t>(GP4_GLOBAL(uint8_t, 0x007ab4a4)) * 0x40u + 0x007ab1a4u;
    uint32_t node = 0x008485a6u;
    for (uint32_t n = 3; n != 0; --n) {
        node = GP4_FIELD(uint32_t, node, 0);
    }
    GP4_FIELD(uint32_t, node, 0xe) = text;
    node = GP4_FIELD(uint32_t, node, 0);
    GP4_FIELD(uint32_t, node, 0xe) = text + 0x20u;

    c.eax = text;
    c.ebx = text + 0x20u;
    c.ecx = 0;
    c.esi = 0x008485a6u;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(text, 0x20u));
    gp4::call_regs(0x0053c8af, c);

    uint32_t index = c.eax;
    if (index >= 2u) {
        index = 1u;
    }
    index <<= 2;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(index, 0x0053f2b8u));
    c.eax = index + 0x0053f2b8u;

    if (index == 0u) {
        const uint32_t slot = GP4_GLOBAL(uint8_t, 0x007ab4a4);
        c.eax = slot * 0x40u + 0x007ab1a4u;
        c.edi = c.eax;
        c.esi = c.edi + 0x40u;
        const uint32_t factor = 0u - slot + 0xbu;
        c.ecx = factor * 0x40u;
        c.eflags = gp4::flags::merge(c.eflags, imulFlags32(static_cast<int32_t>(factor), 0x40));
        const uint32_t count = c.ecx;
        for (uint32_t i = 0; i != count; ++i) {
            GP4_FIELD(uint8_t, c.edi, i) = GP4_FIELD(uint8_t, c.esi, i);
        }
        c.edi += count;
        c.esi += count;
        c.ecx = 0;
        GP4_FIELD(uint8_t, c.edi, 0) = 0u;
        GP4_FIELD(uint8_t, c.edi, 0x20) = 0u;
    }
    *r = c;
}
GP4_IMPL(0x0053f265, FUN_0053f265, "regs(edx:u32, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=case_0054fd4d+anim_0052822a+slots_005388ed+cells_00527eae+slot8_005388ba+surfcopy_00549d55+frame_0082fd70; gfields={0x007ab1a4*192/4:u32, 0x007ab4a4:u8[0..11], 0x008485a6:ptr[0x40]:{*bytes, 0:ptr[0x40]:{*bytes, 0:ptr[0x40]:{*bytes, 0:ptr[0x40]:bytes}}}}; emu=skip:win32 PeekMessageA")
