#include <gp4/gp4.h>
#include <gp4/flags.h>

// Reads two bytes at esi (lodsb x2, esi += 2) as a big-endian word; ax = the word (eax's upper half
// kept) and ebx = the word. After the empty routine 0x0054d6f2, ebx -= eax (the word minus eax, i.e.
// minus the kept upper half), stored as the unaligned dword 0x00833cc5, and the byte 0x00833f74 is set
// to 0xff. The flags are those of the `sub ebx, eax`.
// FUNCTION: GP4 0x0054d341
void __cdecl FUN_0054d341(gp4::Regs* r) {
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t word = (static_cast<uint32_t>(src[0]) << 8) | src[1];
    const uint32_t eax = (r->eax & 0xffff0000u) | word;
    gp4::cdecl_<0x0054d6f2, void>();
    const uint32_t diff = word - eax;
    GP4_GLOBAL(uint32_t, 0x00833cc5) = diff;
    GP4_GLOBAL(uint8_t, 0x00833f74) = 0xffu;
    r->esi = r->esi + 2u;
    r->eax = eax;
    r->ebx = diff;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(word, eax));
}
GP4_IMPL(0x0054d341, FUN_0054d341, "regs(eax:u32, esi:ptr[0x40]:bytes) -> (eax, ebx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
