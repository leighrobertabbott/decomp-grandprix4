#include <gp4/gp4.h>
#include <gp4/flags.h>

// Steps the 16-bit phase [0x66d886] by [0x852de4] and clamps it by the car kind tested in 0x4081fc
// (CF set when byte +0xe4 of the record in esi is 4): kind 4 caps the phase at 0x800 (resetting the
// stored phase to 0x8000), any other kind at 0x400 (stored back as 0x400). The cosine word at index
// (|0x4000 - 16 * phase| >> 2) & 0xfffe of the Q14 table 0x7c5c5c is scaled by half the word
// [0x7a8ea2] (also stored in [0x88f6b8]) and shifted left by 2 into the high word dx; then
// [0x934e38] = [0x8751a4] - (int16)(dx + [0x88f6ba]). eax is that value, edx keeps its high half and
// takes the new dx; the flags are those of the final `add eax, [0x8751a4]`.
// FUNCTION: GP4 0x00552595
void __cdecl FUN_00552595(gp4::Regs* r) {
    const uint16_t step = GP4_GLOBAL(uint16_t, 0x00852de4);
    const uint16_t phase = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x0066d886) + step);
    GP4_GLOBAL(uint16_t, 0x0066d886) = phase;

    gp4::Regs c = *r;
    c.eax = (r->eax & 0xffff0000u) | phase;
    gp4::call_regs(0x004081fc, c);

    int16_t angle = static_cast<int16_t>(phase);
    if ((c.eflags & gp4::flags::CF) == 0u) {
        if (angle >= 0x400) {
            angle = 0x400;
            GP4_GLOBAL(uint16_t, 0x0066d886) = static_cast<uint16_t>(angle);
        }
    } else {
        if (angle >= 0x800) {
            angle = 0x800;
            GP4_GLOBAL(uint16_t, 0x0066d886) = 0x8000u;
        }
    }

    uint16_t index = static_cast<uint16_t>(static_cast<uint16_t>(angle) << 4);
    index = static_cast<uint16_t>(0u - index);
    index = static_cast<uint16_t>(index + 0x4000u);
    if (static_cast<int16_t>(index) < 0) index = static_cast<uint16_t>(0u - index);
    index = static_cast<uint16_t>(index >> 2);
    const int16_t cosine = *reinterpret_cast<const int16_t*>(GP4_ARRAY(const uint8_t, 0x007c5c5c) + (index & 0xfffeu));

    const uint16_t scale = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x007a8ea2) >> 1);
    GP4_GLOBAL(uint16_t, 0x0088f6b8) = scale;
    const int32_t product = static_cast<int32_t>(cosine) * static_cast<int16_t>(scale);
    const uint16_t high = static_cast<uint16_t>((static_cast<uint32_t>(product) << 2) >> 16);

    const uint16_t sum = static_cast<uint16_t>(high + GP4_GLOBAL(uint16_t, 0x0088f6ba));
    const uint32_t negated = 0u - static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(sum)));
    const uint32_t base = GP4_GLOBAL(uint32_t, 0x008751a4);
    const uint32_t result = negated + base;
    GP4_GLOBAL(uint32_t, 0x00934e38) = result;

    r->eax = result;
    r->edx = (r->edx & 0xffff0000u) | high;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(negated, base));
}
GP4_IMPL(0x00552595, FUN_00552595, "regs(esi:ptr[0xec]:u8{0,4,4,1,2,3}, edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
