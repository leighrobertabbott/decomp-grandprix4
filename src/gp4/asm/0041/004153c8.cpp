#include <gp4/gp4.h>

// Translates the control-state bits of the record at esi ([+0x6f] flag bits 1/2/4/8/0x10, [+0xe5] bit 4,
// the word [+0xb4]) into the 0x638d20 / 0x638d30 / 0x638d34 input values (full-scale 0x4000) and the word
// [esi+0xee]. When the byte at 0x603c34 is 0xff, 0x638d20 is [esi+0xb4] * [0x603c38] >> 14 clamped to +-0x4000.
// Leaves eax/edx as the last computation set them and the flags of the final TEST [esi+0x6f], 0x10.
// FUNCTION: GP4 0x004153c8
void __cdecl FUN_004153c8(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;

    GP4_GLOBAL(uint32_t, 0x00638d20) = 0;
    GP4_GLOBAL(uint32_t, 0x00638d30) = 0;
    GP4_GLOBAL(uint32_t, 0x00638d34) = 0;
    GP4_FIELD(uint16_t, rec, 0xee) = 0;

    if ((GP4_FIELD(const uint8_t, rec, 0x6f) & 4) != 0) {
        GP4_GLOBAL(uint32_t, 0x00638d20) = 0xffffc000u;
    }
    if ((GP4_FIELD(const uint8_t, rec, 0x6f) & 8) != 0) {
        GP4_GLOBAL(uint32_t, 0x00638d20) = 0x4000u;
    }

    if (GP4_GLOBAL(const uint8_t, 0x00603c34) == 0xff) {
        const int64_t product = static_cast<int64_t>(static_cast<int32_t>(GP4_FIELD(const int16_t, rec, 0xb4))) *
                                static_cast<int64_t>(GP4_GLOBAL(const int32_t, 0x00603c38));
        edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
        eax = static_cast<uint32_t>(product >> 14);  // SHRD eax, edx, 14
        if (static_cast<int32_t>(eax) < 0) {
            eax = 0u - eax;
            if (eax >= 0x4000u) eax = 0x4000u;
            eax = 0u - eax;
        } else if (eax >= 0x4000u) {
            eax = 0x4000u;
        }
        GP4_GLOBAL(uint32_t, 0x00638d20) = eax;
    }

    if ((GP4_FIELD(const uint8_t, rec, 0x6f) & 2) != 0) {
        eax = 0x4000u;
        if ((GP4_FIELD(const uint8_t, rec, 0xe5) & 4) != 0) {
            eax = GP4_GLOBAL(const uint32_t, 0x00601d40);
        }
        GP4_GLOBAL(uint32_t, 0x00638d30) = eax;
    }

    if ((GP4_FIELD(const uint8_t, rec, 0x6f) & 1) != 0) {
        GP4_FIELD(uint16_t, rec, 0xee) = 0x4000;
    }

    const uint32_t last = GP4_FIELD(const uint8_t, rec, 0x6f) & 0x10u;  // final TEST: PF/ZF set for a zero result
    if (last != 0) {
        GP4_GLOBAL(uint32_t, 0x00638d34) = 0x4000u;
    }

    r->eax = eax;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | (last == 0u ? 0x44u : 0u);
}
GP4_IMPL(0x004153c8, FUN_004153c8, "regs(eax:u32, edx:u32, esi:ptr[0xf6]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
