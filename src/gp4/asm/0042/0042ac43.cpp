#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 16-bit `cmp left, right` / `sub left, right` (CF PF AF ZF SF OF).
static inline uint32_t subFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

struct Unknown_0042ac43_Subject {
    uint8_t Unknown00[0x90];
    int16_t Unknown90;
    uint8_t Unknown92[0xe0 - 0x92];
    int16_t UnknownE0;
};

struct Unknown_0042ac43_Other {
    uint8_t Unknown00[0x90];
    int16_t Unknown90;
};

// When |[esi+0x90] - [ebx+0x90]| is below [0x00603aec] and [0x00603af4] - ax is not negative,
// stores into [esi+0xe0] a doubly scaled copy of that difference clamped to [0x00603af8].
// edx and ecx keep their high halves on the early exits; eax is preserved.
// FUNCTION: GP4 0x0042ac43
void __cdecl FUN_0042ac43(gp4::Regs* r) {
    Unknown_0042ac43_Subject* subject = reinterpret_cast<Unknown_0042ac43_Subject*>(static_cast<uintptr_t>(r->esi));
    const Unknown_0042ac43_Other* other = reinterpret_cast<const Unknown_0042ac43_Other*>(static_cast<uintptr_t>(r->ebx));

    uint16_t dx = static_cast<uint16_t>(static_cast<uint16_t>(subject->Unknown90) - static_cast<uint16_t>(other->Unknown90));
    if (static_cast<int16_t>(dx) < 0) {
        dx = static_cast<uint16_t>(0u - dx);
    }
    r->edx = (r->edx & 0xffff0000u) | dx;

    const uint16_t spread = GP4_GLOBAL(uint16_t, 0x00603aec);
    if (static_cast<int16_t>(dx) >= static_cast<int16_t>(spread)) {
        r->eflags = (r->eflags & ~0x8d5u) | subFlags16(dx, spread);
        return;
    }

    const uint16_t start = GP4_GLOBAL(uint16_t, 0x00603af4);
    const uint16_t ax = static_cast<uint16_t>(r->eax);
    uint16_t cx = static_cast<uint16_t>(start - ax);
    r->ecx = (r->ecx & 0xffff0000u) | cx;
    if (static_cast<int16_t>(cx) < 0) {
        r->eflags = (r->eflags & ~0x8d5u) | subFlags16(start, ax);
        return;
    }

    const uint16_t top = GP4_GLOBAL(uint16_t, 0x00603af8);
    if (static_cast<int16_t>(cx) > static_cast<int16_t>(top)) {
        cx = top;
        r->ecx = (r->ecx & 0xffff0000u) | cx;
    }

    // movsx eax, cx ; imul [0x601c64] ; shrd eax, edx, 14 ; imul [0x603854] ; shrd eax, edx, 16
    const int64_t first = static_cast<int64_t>(static_cast<int32_t>(static_cast<int16_t>(cx))) * GP4_GLOBAL(int32_t, 0x00601c64);
    const int32_t scaled = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(first) >> 14));
    const int64_t second = static_cast<int64_t>(scaled) * GP4_GLOBAL(int32_t, 0x00603854);
    const uint32_t result = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 16);
    subject->UnknownE0 = static_cast<int16_t>(result);

    // flags of the final SHRD by 16 (pinned oracle model: AF = 0, CF = last bit out,
    // OF = bit31(value shifted by 15 ^ result))
    const uint32_t previous = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 15);
    const uint32_t flags = ((previous & 1u) != 0u ? 1u : 0u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((((previous ^ result) >> 31) & 1u) != 0u ? 0x800u : 0u);

    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042ac43, FUN_0042ac43, "regs(eax:u32, ecx:u32, edx:u32, ebx:ptr[0x98]:bytes, esi:ptr[0xe8]:bytes) -> (ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
