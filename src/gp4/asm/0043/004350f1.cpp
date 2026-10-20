#include <gp4/gp4.h>

// One "imul ; shrd eax, edx, N" step: low dword of (a * b) >> N on the signed 64-bit product.
template <int N>
static inline int32_t mulShr(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> N));
}

// High dword (edx) of the signed 64-bit product left behind by "imul".
static inline int32_t mulHigh(int32_t a, int32_t b) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<int32_t>(static_cast<uint64_t>(product) >> 32);
}

static inline int32_t clampByte(int32_t v) {
    if (v < 0) {
        v = 0;
    }
    if (v > 0xff) {
        v = 0xff;
    }
    return v;
}

// FUNCTION: GP4 0x004350f1
// Same scheme as FUN_004351c2 for the two byte pairs of the dword at +0x2a4: the high pair (bits 16..23)
// uses the blend word at +0x1e2, the low pair (bits 0..7) uses 0x4000 - that word. The speed word at
// +0x9a and the word at +0x24a (limited to 0x3800) are scaled by DAT_00601c78/7c/80 and DAT_00603854.
// Each clamped byte is written twice (mov ah, al): the dword becomes c1 c1 c2 c2 (high to low).
// On exit eax = c2 * 0x101, ecx = the stored dword, edx = high half of the last imul, ebp = 0x4000 - the
// word at +0x1e2, and the flags are those of the final "or cx, ax". ebx and edi are preserved.
void __cdecl FUN_004350f1(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    const uint32_t packed = GP4_FIELD(uint32_t, body, 0x2a4);
    const int32_t oldHigh = static_cast<int32_t>((static_cast<int32_t>(packed) >> 16) & 0xff);
    const int32_t oldLow = static_cast<int32_t>(packed & 0xff);

    const int32_t speed = GP4_FIELD(int16_t, body, 0x9a);
    const uint32_t offset = static_cast<uint32_t>(mulShr<14>(GP4_GLOBAL(int32_t, 0x00601c80), speed)) + GP4_GLOBAL(uint32_t, 0x00601c7c);

    const uint32_t blendWord = GP4_FIELD(uint16_t, body, 0x1e2);

    int32_t limited = GP4_FIELD(uint16_t, body, 0x24a);
    if (limited > 0x3800) {
        limited = 0x3800;
    }
    const int32_t scale = mulShr<14>(mulShr<14>(limited, GP4_GLOBAL(int32_t, 0x00601c78)), speed);

    int32_t value = mulShr<14>(scale, static_cast<int32_t>(blendWord));
    value = static_cast<int32_t>(static_cast<uint32_t>(value) - offset);
    value = mulShr<16>(value, GP4_GLOBAL(int32_t, 0x00603854));
    value = static_cast<int32_t>(static_cast<uint32_t>(value) + static_cast<uint32_t>(oldHigh));
    const uint32_t high = static_cast<uint32_t>(clampByte(value)) * 0x101u;

    const uint32_t blend = 0x4000u - blendWord;
    value = mulShr<14>(scale, static_cast<int32_t>(blend));
    value = static_cast<int32_t>(static_cast<uint32_t>(value) - offset);
    const int32_t hi = mulHigh(value, GP4_GLOBAL(int32_t, 0x00603854));
    value = mulShr<16>(value, GP4_GLOBAL(int32_t, 0x00603854));
    value = static_cast<int32_t>(static_cast<uint32_t>(value) + static_cast<uint32_t>(oldLow));
    const uint32_t low = static_cast<uint32_t>(clampByte(value)) * 0x101u;

    const uint32_t result = (high << 16) | low;
    GP4_FIELD(uint32_t, body, 0x2a4) = result;

    // Final "or cx, ax": CF = OF = AF = 0; ZF/SF/PF from the 16-bit result (= low).
    uint32_t flags = 0;
    if (low == 0) {
        flags |= 0x40u;
    }
    if (low & 0x8000u) {
        flags |= 0x80u;
    }
    uint32_t bits = low & 0xffu;
    bits ^= bits >> 4;
    bits ^= bits >> 2;
    bits ^= bits >> 1;
    if (!(bits & 1u)) {
        flags |= 0x04u;
    }

    r->eax = low;
    r->ecx = result;
    r->edx = static_cast<uint32_t>(hi);
    r->ebp = blend;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004350f1, FUN_004350f1, "regs(esi:ptr[0x2b0]:bytes) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
