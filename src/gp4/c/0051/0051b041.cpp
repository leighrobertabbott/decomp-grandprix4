#include <gp4/gp4.h>
#include <gp4/flags.h>

// Converts three 16-bit channel words (0x00933a14, 0x00933a16, 0x00933a18) to bytes at edi, edi+1
// and edi+2: ((w + 0x20) & 0xffff) >> 6, clamped to 0xff. eax is saved and restored by the
// original; the flags are those of the last `cmp ax, 0xff` (on the unclamped third value; the
// clamping `mov` does not touch them).
// FUNCTION: GP4 0x0051b041
void __cdecl FUN_0051b041(gp4::Regs* r) {
    uint8_t* out = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    uint32_t value = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x00933a14) + 0x20u) >> 6;
    if (static_cast<int16_t>(value) > 0xff) {
        value = 0xffu;
    }
    out[0] = static_cast<uint8_t>(value);

    value = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x00933a16) + 0x20u) >> 6;
    if (static_cast<int16_t>(value) > 0xff) {
        value = 0xffu;
    }
    out[1] = static_cast<uint8_t>(value);

    value = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x00933a18) + 0x20u) >> 6;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub16(value, 0xffu));
    if (static_cast<int16_t>(value) > 0xff) {
        value = 0xffu;
    }
    out[2] = static_cast<uint8_t>(value);
}
GP4_IMPL(0x0051b041, FUN_0051b041, "regs(edi:ptr[0x10]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
