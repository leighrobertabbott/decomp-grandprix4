#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0051b123
// Converts one 8-bit RGB triple at esi to 6-bit channels at edi: out = min((in + 2) >> 2, 0x3f) for
// byte 0, 2 and 1 in that order (the original loads, converts and stores them in that order). eax is
// saved and restored; the flags are those of the last `cmp ax, 0x3f` on the unclamped value of byte 1
// (the clamping `mov` does not touch them).
void __cdecl FUN_0051b123(gp4::Regs* r) {
    const uint8_t* const in = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* const out = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    uint32_t value = static_cast<uint16_t>(in[0] + 2u) >> 2;
    out[0] = static_cast<uint8_t>(value > 0x3fu ? 0x3fu : value);

    value = static_cast<uint16_t>(in[2] + 2u) >> 2;
    out[2] = static_cast<uint8_t>(value > 0x3fu ? 0x3fu : value);

    value = static_cast<uint16_t>(in[1] + 2u) >> 2;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub16(value, 0x3fu));
    out[1] = static_cast<uint8_t>(value > 0x3fu ? 0x3fu : value);
}
GP4_IMPL(0x0051b123, FUN_0051b123, "regs(esi:ptr[0x10]:bytes, edi:ptr[0x10]:bytes) -> (cf, pf, af, zf, sf, of)")
