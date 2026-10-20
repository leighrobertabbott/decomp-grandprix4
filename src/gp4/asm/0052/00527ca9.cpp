#include <gp4/gp4.h>
#include <gp4/flags.h>

// Writes the low 16 bits of eax at [edi] as digits '0' + nibble (no hex letters), most significant first, with
// leading zero nibbles skipped (the last nibble is always written), followed by a zero byte. edi is left at
// the terminating zero. eax, ebx and ecx are saved and restored; the flags are those of the final
// `add al, 0x30` on the last nibble.
// FUNCTION: GP4 0x00527ca9
void __cdecl FUN_00527ca9(gp4::Regs* r) {
    const uint32_t value = r->eax;
    uint8_t* out = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    bool started = false;

    for (uint32_t shift = 12u; shift != 0u; shift -= 4u) {
        const uint32_t digit = (value >> shift) & 0xfu;
        if (digit != 0u || started) {
            *out++ = static_cast<uint8_t>(digit + 0x30u);
            started = true;
        }
    }
    const uint32_t last = value & 0xfu;
    *out++ = static_cast<uint8_t>(last + 0x30u);
    *out = 0;

    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(out));
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add8(last, 0x30u));
}
GP4_IMPL(0x00527ca9, FUN_00527ca9, "regs(eax:u32, edi:ptr[0x10]:bytes) -> (edi, cf, pf, af, zf, sf, of)")
