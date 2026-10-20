#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF PF AF ZF SF OF) of the 16-bit "cmp left, right".
static inline uint32_t compareFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Scaled 16.16 value (DAT_006039f4, or the negated DAT_006039f8 for a record with bit 0x80 at +0xe5)
// times DAT_0060384c, plus the word DAT_00638d6a, clamped to 0..0x4000 and to the word at +0xee of the
// record in esi. Early exit (eax, edx and the flags of "test byte [esi+0x230], 0x40" unchanged) when
// +0xe5 has bits 0x80 and 4, byte +0x51 of the edi record has bit 0x80 and +0x230 has bit 0x40.
// FUNCTION: GP4 0x00415f79
void __cdecl FUN_00415f79(gp4::Regs* r) {
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t* other = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->edi));
    const uint32_t eaxIn = r->eax;
    const uint8_t recordFlags = record[0xe5];

    int32_t value;
    if ((recordFlags & 0x80u) != 0u) {
        if ((recordFlags & 4u) != 0u && (other[0x51] & 0x80u) != 0u && (record[0x230] & 0x40u) != 0u) {
            // test byte [esi+0x230], 0x40 (non-zero result): CF = OF = AF = 0, ZF = SF = PF = 0.
            r->eax = eaxIn;
            r->eflags = r->eflags & ~0x8d5u;
            return;
        }
        value = static_cast<int32_t>(0u - static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x006039f8)));
    } else {
        value = GP4_GLOBAL(int32_t, 0x006039f4);
    }

    const int64_t product = static_cast<int64_t>(value) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0060384c));
    const uint32_t offset = GP4_GLOBAL(uint16_t, 0x00638d6a);
    int32_t result = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> 16) + offset);
    if (result < 0) {
        result = 0;
    }
    if (result > 0x4000) {
        result = 0x4000;
    }

    const uint16_t limit = *reinterpret_cast<const uint16_t*>(record + 0xee);
    uint32_t eax = static_cast<uint32_t>(result);
    const uint32_t flags = compareFlags16(eax, limit);
    if (static_cast<int16_t>(eax) > static_cast<int16_t>(limit)) {
        eax = (eax & 0xffff0000u) | limit;
    }

    r->eax = eax;
    r->edx = offset;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00415f79, FUN_00415f79, "regs(esi:ptr[0x238]:bytes, edi:ptr[0x59]:bytes, eax:u32, edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
