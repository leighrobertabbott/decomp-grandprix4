#include <gp4/gp4.h>
#include <gp4/flags.h>

// Hand-written routine: two Q14 sine/cosine table products of the 16-bit angle word [edi] with a
// signed distance word.
//   distance = (mask & dl) ? -(word[0x870608] - cx) : (word[0x870604] - cx), stored in word 0x00934df0.
//   si = table[|angle|], bx = table[|0x4000 - angle|] (word offsets from 0x007c5c5c, (|a| >> 2) & 0xfffe).
//   dx:ax = (si * distance) >> 14 and cx:bx = (bx * distance) >> 14 (arithmetic, 32-bit products;
//   the sar/rcr loop runs 14 times).  Leaves esi = 0 and the flags of the final `sub si, 1`.
static __declspec(noinline) int16_t TableWord(uint32_t byteOffset) {
    return *reinterpret_cast<const int16_t*>(GP4_ARRAY(uint8_t, 0x007c5c5c) + byteOffset);
}

// FUNCTION: GP4 0x00559663
void __cdecl FUN_00559663(gp4::Regs* r) {
    const uint32_t eaxIn = r->eax;
    const uint32_t ecxIn = r->ecx;
    const uint32_t edxIn = r->edx;
    const uint16_t cx = static_cast<uint16_t>(ecxIn);
    const uint8_t dl = static_cast<uint8_t>(edxIn);
    const uint16_t* angleWord = reinterpret_cast<const uint16_t*>(r->edi);

    uint16_t distance;
    if ((GP4_GLOBAL(uint8_t, 0x0086855c) & dl) != 0) {
        distance = static_cast<uint16_t>(0u - static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x00870608) - cx));
    } else {
        distance = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x00870604) - cx);
    }
    GP4_GLOBAL(uint16_t, 0x00934df0) = distance;

    const uint16_t angle = *angleWord;
    uint16_t a = angle;
    if (static_cast<int16_t>(a) < 0) a = static_cast<uint16_t>(0u - a);
    const int16_t sineWord = TableWord((static_cast<uint32_t>(a) >> 2) & 0xfffeu);

    uint16_t b = static_cast<uint16_t>(0x4000u - angle);
    if (static_cast<int16_t>(b) < 0) b = static_cast<uint16_t>(0u - b);
    const int16_t cosineWord = TableWord((static_cast<uint32_t>(b) >> 2) & 0xfffeu);

    const int32_t q1 = (static_cast<int32_t>(cosineWord) * static_cast<int32_t>(static_cast<int16_t>(distance))) >> 14;
    const int32_t q2 = (static_cast<int32_t>(sineWord) * static_cast<int32_t>(static_cast<int16_t>(distance))) >> 14;

    r->eax = (eaxIn & 0xffff0000u) | (static_cast<uint32_t>(q2) & 0xffffu);
    r->edx = (edxIn & 0xffff0000u) | ((static_cast<uint32_t>(q2) >> 16) & 0xffffu);
    r->ebx = static_cast<uint32_t>(q1) & 0xffffu;
    r->ecx = (ecxIn & 0xffff0000u) | ((static_cast<uint32_t>(q1) >> 16) & 0xffffu);
    r->esi = 0;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub16(1u, 1u));
}
GP4_IMPL(0x00559663, FUN_00559663, "regs(eax:u32, ecx:u32, edx:u32, edi:ptr[8]:bytes) -> (eax, ebx, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
