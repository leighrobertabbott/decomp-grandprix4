#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x005571e6
// Reads the two words at esi (advancing esi by 4). Only when the mode byte 0x0086857c is 1 are
// they kept: ax = first word, dx = second word (upper halves of eax/edx unchanged), and, by the
// kind in cl, stored as a word pair at 0x0088f04e / 0x0088f052 (kind 0x0f) or at
// 0x0088f07a / 0x0088f07e (any other kind). Flags: `add esi, 4` for any other mode, `cmp cl, 0x0f`
// in mode 1 (the add's flags are overwritten there).
void __cdecl FUN_005571e6(gp4::Regs* r) {
    const uint32_t esi = r->esi;
    r->esi = esi + 4u;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 1) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi, 4u));
        return;
    }
    const uint16_t first = GP4_FIELD(uint16_t, esi, 0);
    const uint16_t second = GP4_FIELD(uint16_t, esi, 2);
    r->eax = (r->eax & 0xffff0000u) | first;
    r->edx = (r->edx & 0xffff0000u) | second;
    const uint32_t kind = r->ecx & 0xffu;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(kind, 0x0fu));
    if (kind == 0x0fu) {
        GP4_GLOBAL(uint16_t, 0x0088f04e) = first;
        GP4_GLOBAL(uint16_t, 0x0088f052) = second;
        return;
    }
    GP4_GLOBAL(uint16_t, 0x0088f07a) = first;
    GP4_GLOBAL(uint16_t, 0x0088f07e) = second;
}
GP4_IMPL(0x005571e6, FUN_005571e6, "regs(eax:u32, edx:u32, ecx:u32{0x0f,0x00,0x57,0xffffff0f,0x10}, esi:ptr[8]:bytes) -> (eax, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
