#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stream decoder opcode "set one of 16 words". When the mode byte 0x86857c is 1 the word at esi + 2
// is stored into the 16-word table 0x00811aa8 at index (word at esi) & 0xf (edx = that index, ax =
// the stored word, the rest of eax is kept); in any mode esi then advances by four and the flags are
// those of that add.
// FUNCTION: GP4 0x00556ec7
void __cdecl FUN_00556ec7(gp4::Regs* r) {
    const uint32_t before = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) == 1u) {
        const uint32_t index = GP4_FIELD(uint16_t, before, 0) & 0xfu;
        const uint32_t value = GP4_FIELD(uint16_t, before, 2);
        GP4_ARRAY(uint16_t, 0x00811aa8)[index] = static_cast<uint16_t>(value);
        r->edx = index;
        r->eax = (r->eax & 0xffff0000u) | value;
    }
    r->esi = before + 4u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(before, 4u));
}
GP4_IMPL(0x00556ec7, FUN_00556ec7, "regs(eax:u32, edx:u32, esi:ptr[8]:bytes) -> (eax, edx, esi, cf, pf, af, zf, sf, of); globals=layout=mode_0086857c")
