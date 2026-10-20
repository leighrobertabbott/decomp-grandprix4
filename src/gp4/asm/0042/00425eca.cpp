#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Record addressed through esi (only the fields this routine reads).
struct Unknown_00425eca_Esi {
    uint8_t Unknown00[0x71];
    uint8_t Flags71;
    uint8_t Unknown72[0x8d - 0x72];
    uint8_t Flags8D;
    uint8_t Unknown8E[0x98 - 0x8e];
    int32_t Unknown98;
    uint8_t Unknown9C[0xfe - 0x9c];
    uint8_t FlagsFE;
    uint8_t UnknownFF[0x25f - 0xff];
    uint8_t Flags25F;
};

// ebx, ecx and edi are saved and restored. Three outcomes: with bit 5 of byte +0xfe the result of
// 0x425e19 (indexed by |dword +0x98| / 4, capped at 0x16600000, >> 20, plus 0x168 for bit 0 of
// byte +0x8d); else with bit 1 of byte +0x25f the product DAT_0062786c * DAT_00603854 >> 2 (edx
// keeps the high half); else the signed word at DAT_006039bc << 14, negated for bit 2 of byte +0x71.
// The flags are those of the last flag writer on each path.
// FUNCTION: GP4 0x00425eca
void __cdecl FUN_00425eca(gp4::Regs* r) {
    const Unknown_00425eca_Esi* rec = reinterpret_cast<const Unknown_00425eca_Esi*>(static_cast<uintptr_t>(r->esi));

    int32_t magnitude = rec->Unknown98;
    if (magnitude < 0) {
        magnitude = -magnitude;
    }
    const uint32_t scaled = static_cast<uint32_t>(magnitude) >> 2;

    uint32_t flags;
    if ((rec->FlagsFE & 0x20u) != 0u) {
        uint32_t index = scaled;
        if (static_cast<int32_t>(index) > 0x16600000) {
            index = 0x16600000u;
        }
        index >>= 0x14;
        if ((rec->Flags8D & 1u) != 0u) {
            index += 0x168u;
        }
        gp4::Regs callee = *r;
        callee.ebx = scaled;
        callee.edi = index;
        callee.st_in = 0;
        callee.st_out = 0;
        gp4::call_regs(0x00425e19, callee);
        r->eax = callee.eax;
        r->edx = callee.edx;
        flags = callee.eflags & 0x8d5u;
    } else if ((rec->Flags25F & 2u) != 0u) {
        const int64_t product = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0062786c))
            * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603854));
        const uint32_t low = static_cast<uint32_t>(product);
        const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
        const uint32_t result = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 2);
        const uint32_t previous = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 1);
        // shrd by 2: CF = last bit shifted out; OF = bit31(previous ^ result), AF = 0 (pinned oracle model)
        flags = ((low >> 1) & 1u)
            | parityFlag(result)
            | (result == 0u ? 0x40u : 0u)
            | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
            | ((((previous ^ result) >> 31) & 1u) != 0u ? 0x800u : 0u);
        r->eax = result;
        r->edx = high;
    } else {
        uint32_t value = static_cast<uint32_t>(static_cast<int32_t>(GP4_GLOBAL(int16_t, 0x006039bc))) << 0xe;
        if ((rec->Flags71 & 4u) != 0u) {
            const uint32_t negated = 0u - value;
            flags = (value != 0u ? 1u : 0u)
                | parityFlag(negated)
                | (((value ^ negated) & 0x10u) != 0u ? 0x10u : 0u)
                | (negated == 0u ? 0x40u : 0u)
                | ((negated & 0x80000000u) != 0u ? 0x80u : 0u)
                | (value == 0x80000000u ? 0x800u : 0u);
            value = negated;
        } else {
            // test byte, 4 with a zero result: PF = 1, ZF = 1
            flags = 0x44u;
        }
        r->eax = value;
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00425eca, FUN_00425eca, "regs(esi:ptr[0x267]:bytes, edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
