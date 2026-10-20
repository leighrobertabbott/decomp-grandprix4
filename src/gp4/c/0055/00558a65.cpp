#include <gp4/gp4.h>
#include <gp4/flags.h>

extern "C" void __cdecl __debugbreak(void);

// Stream decoder opcode with no payload. When the mode byte 0x86857c is not 2, esi just advances by
// two (flags of that add). In mode 2 the word at esi is read into ax (the rest of eax is kept), esi
// advances by two and the word must be 4 (cmp ax, 4: the flags of that compare): 0x868584 is then set
// to 0xff; any other word runs into the int3 assertion.
// FUNCTION: GP4 0x00558a65
void __cdecl FUN_00558a65(gp4::Regs* r) {
    const uint32_t before = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 2) {
        r->esi = before + 2u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(before, 2u));
        return;
    }
    const uint32_t word = GP4_FIELD(uint16_t, before, 0);
    r->eax = (r->eax & 0xffff0000u) | word;
    r->esi = before + 2u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub16(word, 4u));
    if (word != 4u) {
        __debugbreak();
        return;
    }
    GP4_GLOBAL(uint8_t, 0x00868584) = 0xffu;
}
GP4_IMPL(0x00558a65, FUN_00558a65, "regs(eax:u32, esi:ptr[8]:u16{=4}) -> (eax, esi, cf, pf, af, zf, sf, of); globals=layout=mode_0086857c")
