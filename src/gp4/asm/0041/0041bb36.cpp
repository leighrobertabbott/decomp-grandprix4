#include <gp4/gp4.h>

struct Unknown_0041bb36 {
    uint8_t Unknown00[0x72];
    int8_t Selector72;                    // signed: sign flips the divisor, magnitude indexes the table
    uint8_t Unknown73[0x1c6 - 0x73];
    uint8_t Table1C6[127];
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x0041bb36
// Q14 fixed point: step = +-(0x100000 / table[max(selector, 1) - 1]) (negated for a negative selector),
// then 0x623b68 = step, 0x623b6c = g * step >> 14, 0x623b70 = that squared >> 14 and
// 0x623b74 = g * g >> 14, with g = [0x601330]. Leaves eax = 0x623b74, edx = high dword of g * g,
// ebp = the divisor byte; the flags are those of the final "shrd eax, edx, 14" (pinned oracle
// model: AF = 0, CF = bit 13 of the product, OF = bit31(product>>13 ^ product>>14)).
void __cdecl FUN_0041bb36(gp4::Regs* r) {
    const Unknown_0041bb36* rec = reinterpret_cast<const Unknown_0041bb36*>(static_cast<uintptr_t>(r->esi));

    int32_t index = rec->Selector72;
    if (index < 1) {
        index = 1;
    }
    const uint32_t divisor = rec->Table1C6[index - 1];
    uint32_t step = 0x100000u / divisor;
    if (rec->Selector72 < 0) {
        step = 0u - step;
    }
    GP4_GLOBAL(uint32_t, 0x00623b68) = step;

    const int32_t base = GP4_GLOBAL(int32_t, 0x00601330);
    const int64_t scaled = static_cast<int64_t>(base) * static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00623b68));
    const uint32_t scaledQ14 = static_cast<uint32_t>(static_cast<uint64_t>(scaled) >> 14);
    GP4_GLOBAL(uint32_t, 0x00623b6c) = scaledQ14;

    const int64_t squared = static_cast<int64_t>(static_cast<int32_t>(scaledQ14)) * static_cast<int32_t>(scaledQ14);
    GP4_GLOBAL(uint32_t, 0x00623b70) = static_cast<uint32_t>(static_cast<uint64_t>(squared) >> 14);

    const int32_t baseAgain = GP4_GLOBAL(int32_t, 0x00601330);
    const int64_t baseSquared = static_cast<int64_t>(baseAgain) * baseAgain;
    const uint32_t result = static_cast<uint32_t>(static_cast<uint64_t>(baseSquared) >> 14);
    const uint32_t previous = static_cast<uint32_t>(static_cast<uint64_t>(baseSquared) >> 13);
    GP4_GLOBAL(uint32_t, 0x00623b74) = result;

    const uint32_t flags = (previous & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((previous ^ result) >> 31) != 0u) ? 0x800u : 0u);

    r->eax = result;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(baseSquared) >> 32);
    r->ebp = divisor;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041bb36, FUN_0041bb36, "regs(esi:ptr[0x250]:layout=rec_0041bb36) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
