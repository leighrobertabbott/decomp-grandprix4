#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stream command handler (mode byte 0x0086857c): in any mode but 1 the cursor esi skips a word (flags
// of that `add esi, 2`). In mode 1 the word at esi is read into ax, esi advances by two and ax becomes
// (word - 1) & 7. cl == 0x19 selects the second target: al << 3 is stored in the byte 0x00868580
// (flags of that 8-bit shift, CF = OF = 0 since al <= 7); any other cl stores al in the byte
// 0x0086857e (flags of `cmp cl, 0x19`).
// FUNCTION: GP4 0x005572ed
void __cdecl FUN_005572ed(gp4::Regs* r) {
    const uint32_t esi = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 1u) {
        r->esi = esi + 2u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi, 2u));
        return;
    }
    const uint32_t word = GP4_FIELD(uint16_t, esi, 0);
    const uint32_t index = (word - 1u) & 7u;
    r->esi = esi + 2u;
    const uint32_t kind = r->ecx & 0xffu;
    if (kind != 0x19u) {
        r->eax = (r->eax & 0xffff0000u) | index;
        GP4_GLOBAL(uint8_t, 0x0086857e) = static_cast<uint8_t>(index);
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(kind, 0x19u));
        return;
    }
    const uint32_t shifted = (index << 3) & 0xffu;
    r->eax = (r->eax & 0xffff0000u) | shifted;
    GP4_GLOBAL(uint8_t, 0x00868580) = static_cast<uint8_t>(shifted);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(shifted));
}
GP4_IMPL(0x005572ed, FUN_005572ed, "regs(eax:u32, ecx:u32{0x19,0x19,0,1,0x119,0xff}, esi:ptr[8]:bytes) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_0086857c")
