#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the mode byte 0x0086857c is 2, takes the 11-bit field (word at +0x1a, & 0x7ff) of the record
// at ebx and files it by the kind in cl: kind 0x17 stores the record pointer / field in the second
// slot pair (0x0086851c / 0x00868566), any other kind in the first pair (0x00868518 / 0x00868564).
// Only ax is changed in eax; the flags are those of `cmp cl, 0x17` (mode 2) or `add esi, 0`.

// FUNCTION: GP4 0x00557292
void __cdecl FUN_00557292(gp4::Regs* r) {
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 2) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(r->esi, 0u));
        return;
    }
    const uint32_t rec = r->ebx;
    const uint32_t kind = r->ecx & 0xffu;
    const uint16_t field = (uint16_t)(GP4_FIELD(uint16_t, rec, 0x1a) & 0x7ffu);
    r->eax = (r->eax & 0xffff0000u) | field;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(kind, 0x17u));
    if (kind != 0x17u) {
        GP4_GLOBAL(uint32_t, 0x00868518) = rec;
        GP4_GLOBAL(uint16_t, 0x00868564) = field;
        return;
    }
    GP4_GLOBAL(uint32_t, 0x0086851c) = rec;
    GP4_GLOBAL(uint16_t, 0x00868566) = field;
}
GP4_IMPL(0x00557292, FUN_00557292, "regs(eax:u32, ecx:u32{0x17,0x16,0x00,0x57,0xffffff17}, ebx:ptr[0x24]:bytes, esi:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
