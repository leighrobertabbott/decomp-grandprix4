#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of `cmp left, right`.
static inline uint32_t cmpFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

// Picks the 16-byte entry of the table at 0x868598 selected by a word of the block at edi (+0xaa
// when edx == 3, else +0x132) and compares ecx with its four breakpoints (+0, +4, +8, +0xc).
// ecx past the last breakpoint returns eax = 0 with CF set (flags of the compare). Otherwise the
// segment (offset, +4 or +8) is interpolated by 0x4031ed, ebx = 1, or 5 when ecx - 0x18 is also
// beyond the third breakpoint, and the flag byte 0x625aba is set to 0xff in the other cases.
// FUNCTION: GP4 0x00403263
void __cdecl FUN_00403263(gp4::Regs* r) {
    const uint8_t* block = reinterpret_cast<const uint8_t*>(r->edi);
    const uint32_t selector = r->edx;
    const int32_t value = static_cast<int32_t>(r->ecx);

    uint32_t index = *reinterpret_cast<const uint16_t*>(block + 0xaa);
    if (selector != 3u) {
        index = *reinterpret_cast<const uint16_t*>(block + 0x132);
    }
    const uint32_t offset = index * 16u;

    const int32_t limit = GP4_FIELD(int32_t, GP4_ARRAY(uint8_t, 0x008685a4), offset);
    if (value > limit) {
        r->eax = 0u;
        r->ebx = 1u;
        r->eflags = (r->eflags & ~0x8d5u) | cmpFlags(r->ecx, static_cast<uint32_t>(limit)) | 1u;
        return;
    }

    uint32_t segment = offset;
    uint32_t level = 1u;
    bool mark = true;
    if (value > GP4_FIELD(int32_t, GP4_ARRAY(uint8_t, 0x0086859c), offset)) {
        if (value <= GP4_FIELD(int32_t, GP4_ARRAY(uint8_t, 0x008685a0), offset)) {
            segment = offset + 4u;
        } else {
            segment = offset + 8u;
            if (value - 0x18 > GP4_FIELD(int32_t, GP4_ARRAY(uint8_t, 0x008685a0), offset)) {
                level = 5u;
                mark = false;
            }
        }
    }
    if (mark) {
        GP4_GLOBAL(uint8_t, 0x00625aba) = 0xffu;
    }

    gp4::Regs callee = *r;
    callee.esi = segment;
    callee.ebx = level;
    gp4::call_regs(0x004031ed, callee);

    r->eax = callee.eax;
    r->ebx = level;
    r->eflags = (r->eflags & ~0x8d5u) | (callee.eflags & 0x8d4u);
}
GP4_IMPL(0x00403263, FUN_00403263, "regs(edi:ptr[0x13a]:bytes, edx:u32[2..3], ecx:i32) -> (eax, ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
