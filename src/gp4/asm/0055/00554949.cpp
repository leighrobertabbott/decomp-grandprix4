#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00554949
// bh holds two nibbles; the byte [0x874edf] is added to each (8-bit wrap-around), both are run
// through the clamp 0x005548cf and the results are repacked into bh (high nibble from the high
// half). eax, ecx and edx are saved and restored; the flags are those of the final
// `or al, ah` (logic flags of the repacked byte).
void __cdecl FUN_00554949(gp4::Regs* r) {
    const uint8_t packed = static_cast<uint8_t>(r->ebx >> 8);
    const uint8_t bias = GP4_GLOBAL(uint8_t, 0x00874edf);

    const uint8_t lowIn = static_cast<uint8_t>((packed & 0xfu) + bias);
    const uint8_t highIn = static_cast<uint8_t>((packed >> 4) + bias);

    gp4::Regs lowCall = {};
    lowCall.ecx = lowIn;
    gp4::call_regs(0x005548cf, lowCall);
    const uint8_t low = static_cast<uint8_t>(lowCall.ecx);

    gp4::Regs highCall = {};
    highCall.ecx = highIn;
    gp4::call_regs(0x005548cf, highCall);
    const uint8_t high = static_cast<uint8_t>(highCall.ecx);

    const uint8_t result = static_cast<uint8_t>((low & 0xfu) | static_cast<uint8_t>(high << 4));

    r->ebx = (r->ebx & 0xffff00ffu) | (static_cast<uint32_t>(result) << 8);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(result));
}
GP4_IMPL(0x00554949, FUN_00554949, "regs(ebx:u32) -> (ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
