#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0054b978
// Builds one network/packet slot: slot = DAT_007aafd0 & 7 selects one of eight 0x104-byte records
// at 0x7aa7b0. The record gets the length dword DAT_007a97ac, a tag byte (0xe0 | slot) and then
// length-1 bytes copied from 0x7a97b1 (the byte at 0x7a97b0 is skipped).
// Ends with esi/edi advanced, ecx = 0, eax = length with its low byte replaced by the tag; the
// flags are those of `dec ecx` (CF kept clear by the preceding `or al, 0xe0`).
void __cdecl FUN_0054b978(gp4::Regs* r) {
    const uint32_t length = GP4_GLOBAL(uint32_t, 0x007a97ac);
    const uint32_t slot = GP4_GLOBAL(uint8_t, 0x007aafd0) & 7u;
    const uint32_t tag = slot | 0xe0u;

    uint8_t* dst = GP4_ARRAY(uint8_t, 0x007aa7b0) + slot * 0x104u;
    const uint8_t* src = GP4_ARRAY(uint8_t, 0x007a97b1);

    *reinterpret_cast<uint32_t*>(dst) = length;
    dst += 4;
    *dst++ = static_cast<uint8_t>(tag);

    const uint32_t count = length - 1u;
    for (uint32_t i = 0; i < count; ++i) {
        *dst++ = *src++;
    }

    r->eax = (length & 0xffffff00u) | tag;
    r->ecx = 0u;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(src));
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(dst));
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(length, 0u));
}
GP4_IMPL(0x0054b978, FUN_0054b978, "regs() -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0054b978")
