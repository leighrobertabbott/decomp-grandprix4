#include <gp4/gp4.h>
#include <gp4/flags.h>

// Advances esi past an 8-byte entry (four words). When the byte 0x0086857c is 1 it also leaves eax = (low byte of
// the word +0) * 3 and dx = the word +6 (the words +2 and +4 are loaded into dx first and overwritten); edi is
// pushed and popped. The flags are those of the final `add esi, 8`.
// FUNCTION: GP4 0x005574aa
void __cdecl FUN_005574aa(gp4::Regs* r) {
    const uint32_t esi = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) == 1u) {
        const uint16_t* entry = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(esi));
        r->eax = (entry[0] & 0xffu) * 3u;
        r->edx = (r->edx & 0xffff0000u) | entry[3];
    }
    r->esi = esi + 8u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi, 8u));
}
GP4_IMPL(0x005574aa, FUN_005574aa, "regs(eax:u32, edx:u32, esi:ptr[0x10]:bytes) -> (eax, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
