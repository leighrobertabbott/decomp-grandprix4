#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stream command handler in mode 2 ([0x86857c]); any other mode just skips a dword (flags of that
// `add esi, 4`). In mode 2 two words are read from esi: the first (masked with 0x3f, left in edx) and
// the second (left in ax). With cl == 0x3f they are stored in the record ebx at +0x19 and +0x17,
// otherwise at +0x18 and +0x16 (low bytes). The flags are those of `cmp cl, 0x3f`.
// FUNCTION: GP4 0x005591ca
void __cdecl FUN_005591ca(gp4::Regs* r) {
    const uint32_t esi = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 2u) {
        r->esi = esi + 4u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi, 4u));
        return;
    }
    const uint32_t first = GP4_FIELD(uint16_t, esi, 0) & 0x3fu;
    const uint32_t second = GP4_FIELD(uint16_t, esi, 2);
    r->esi = esi + 4u;
    r->edx = first;
    r->eax = (r->eax & 0xffff0000u) | second;
    const uint32_t kind = r->ecx & 0xffu;
    if (kind != 0x3fu) {
        GP4_FIELD(uint8_t, r->ebx, 0x18) = static_cast<uint8_t>(first);
        GP4_FIELD(uint8_t, r->ebx, 0x16) = static_cast<uint8_t>(second);
    } else {
        GP4_FIELD(uint8_t, r->ebx, 0x19) = static_cast<uint8_t>(first);
        GP4_FIELD(uint8_t, r->ebx, 0x17) = static_cast<uint8_t>(second);
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(kind, 0x3fu));
}
GP4_IMPL(0x005591ca, FUN_005591ca, "regs(eax:u32, ecx:u32{0x3f,0x3f,0,1,0x13f,0xff}, ebx:ptr[0x20]:bytes, esi:ptr[8]:bytes) -> (eax, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_0086857c")
