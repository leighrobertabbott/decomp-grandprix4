#include <gp4/gp4.h>

// One "imul ; shrd eax, edx, N" step: low dword of (a * b) >> N on the signed 64-bit product.
template <int N>
static inline int32_t mulShr(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> N));
}

// FUNCTION: GP4 0x004351c2
// Updates the four saturating bytes at +0x2a4..+0x2a7 from the dwords at +0x2ec..+0x2f8, the word at
// +0x1e2 (taken as 0x4000 - w for the first two entries), the word at +0x9a and the word at +0x24a
// (limited to 0x3800), scaled by DAT_00601c78/7c/80 and DAT_00603854. On exit ecx = 4, edx = the old
// byte at +0x2a7, ebp = the word at +0x1e2, eax = the last stored byte and the flags are those of
// the final "cmp ecx, 4" (ZF and PF set). ebx and edi are preserved.
void __cdecl FUN_004351c2(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    const int32_t speed = GP4_FIELD(int16_t, body, 0x9a);
    const uint32_t offset = static_cast<uint32_t>(mulShr<14>(GP4_GLOBAL(int32_t, 0x00601c80), speed)) + GP4_GLOBAL(uint32_t, 0x00601c7c);

    int32_t limited = GP4_FIELD(uint16_t, body, 0x24a);
    if (limited > 0x3800) {
        limited = 0x3800;
    }
    const int32_t scale = mulShr<14>(mulShr<14>(limited, GP4_GLOBAL(int32_t, 0x00601c78)), speed);

    const uint32_t blendWord = GP4_FIELD(uint16_t, body, 0x1e2);
    uint32_t blend = blendWord;
    uint32_t oldByte = 0;
    int32_t value = 0;
    for (int index = 0; index != 4; ++index) {
        blend = blendWord;
        if (index < 2) {
            blend = 0x4000u - blend;
        }
        int32_t entry = GP4_FIELD(int32_t, body, 0x2ec + index * 4) >> 1;
        if (entry > 0x4000) {
            entry = 0x4000;
        }
        value = mulShr<14>(entry, scale);
        value = mulShr<14>(value, static_cast<int32_t>(blend));
        value = static_cast<int32_t>(static_cast<uint32_t>(value) - offset);
        value = mulShr<16>(value, GP4_GLOBAL(int32_t, 0x00603854));
        oldByte = GP4_FIELD(uint8_t, body, 0x2a4 + index);
        value = static_cast<int32_t>(static_cast<uint32_t>(value) + oldByte);
        if (value < 0) {
            value = 0;
        }
        if (value > 0xff) {
            value = 0xff;
        }
        GP4_FIELD(uint8_t, body, 0x2a4 + index) = static_cast<uint8_t>(value);
    }

    r->eax = static_cast<uint32_t>(value);
    r->ecx = 4;
    r->edx = oldByte;
    r->ebp = blend;
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x004351c2, FUN_004351c2, "regs(esi:ptr[0x304]:bytes) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
