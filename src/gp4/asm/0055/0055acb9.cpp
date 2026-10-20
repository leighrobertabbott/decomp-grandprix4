#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies the 16 bytes at esi into both 16-byte buffers at 0x007c4164 and 0x007c4174 (each byte is
// loaded from esi once per buffer), then patches byte 2 of each with 0xb2 and byte 9 with 0xb5.
// Leaves esi += 16, ebx = 0x7c4164, edi = 0x7c4174, ecx = 0, al = the last source byte and the
// flags of the final `inc edi` (CF as on entry).

// FUNCTION: GP4 0x0055acb9
void __cdecl FUN_0055acb9(gp4::Regs* r) {
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* first = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(0x007c4164u));
    uint8_t* second = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(0x007c4174u));

    uint8_t value = 0;
    for (int left = 16; left != 0; --left) {
        value = *src;
        *first = value;
        value = *src;
        *second = value;
        ++src;
        ++first;
        ++second;
    }
    uint32_t flags = gp4::flags::inc32(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(second - 1)), r->eflags);

    GP4_FIELD(uint8_t, 0x007c4164u, 2) = 0xb2;
    GP4_FIELD(uint8_t, 0x007c4174u, 2) = 0xb2;
    GP4_FIELD(uint8_t, 0x007c4164u, 9) = 0xb5;
    GP4_FIELD(uint8_t, 0x007c4174u, 9) = 0xb5;

    r->eax = (r->eax & 0xffffff00u) | value;
    r->ebx = 0x007c4164u;
    r->ecx = 0;
    r->edi = 0x007c4174u;
    r->esi += 16;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0055acb9, FUN_0055acb9, "regs(eax:u32, esi:ptr[0x40]:bytes, cf:u32[0..1]) -> (eax, ebx, ecx, esi, edi, cf, pf, af, zf, sf, of)")
