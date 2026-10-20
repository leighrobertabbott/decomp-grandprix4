#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 16-bit `sub left, right` / `cmp left, right` / `neg right` (left = 0): CF PF AF ZF SF OF.
static inline uint32_t subFlags16(uint16_t left, uint16_t right) {
    const uint16_t result = static_cast<uint16_t>(left - right);
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

struct Unknown_0042ac9c_Subject {
    uint8_t Unknown00[0x90];
    int16_t Unknown90;
    uint8_t Unknown92[0x11c - 0x92];
    int16_t Unknown11C;
    int16_t Unknown11E;
};

struct Unknown_0042ac9c_Other {
    uint8_t Unknown00[0x90];
    int16_t Unknown90;
};

// Stores into [0x00604154] the high byte of either the negated [ebx+0x90] (when the +-0x300 window
// test against [ebx+0x11c]/[ebx+0x11e] fails) or of [esi+0x90] - [ebx+0x90] (when it holds).
// edx keeps its high half; the flags are those of the final neg/sub.
// FUNCTION: GP4 0x0042ac9c
void __cdecl FUN_0042ac9c(gp4::Regs* r) {
    const Unknown_0042ac9c_Subject* self = reinterpret_cast<const Unknown_0042ac9c_Subject*>(static_cast<uintptr_t>(r->ebx));
    const Unknown_0042ac9c_Other* other = reinterpret_cast<const Unknown_0042ac9c_Other*>(static_cast<uintptr_t>(r->esi));

    const int16_t value = self->Unknown90;
    const int16_t limitA = self->Unknown11C;
    const int16_t limitB = self->Unknown11E;

    // First/second bound depend on the sign of the value.
    const int16_t firstLimit = value < 0 ? limitA : limitB;
    const int16_t secondLimit = value < 0 ? limitB : limitA;

    uint16_t dx = static_cast<uint16_t>(value);
    if (value < 0) {
        dx = static_cast<uint16_t>(0u - dx);
    }
    dx = static_cast<uint16_t>(dx + 0x300u);

    bool inWindow = false;
    if (!(static_cast<int16_t>(dx) > firstLimit)) {
        dx = static_cast<uint16_t>(dx - 0x600u);
        if (static_cast<int16_t>(dx) >= 0) {
            inWindow = true;
        } else {
            dx = static_cast<uint16_t>(0u - dx);
            inWindow = static_cast<int16_t>(dx) <= secondLimit;
        }
    }

    uint16_t result;
    uint32_t flags;
    if (inWindow) {
        const uint16_t left = static_cast<uint16_t>(other->Unknown90);
        const uint16_t right = static_cast<uint16_t>(value);
        result = static_cast<uint16_t>(left - right);
        flags = subFlags16(left, right);
    } else {
        const uint16_t right = static_cast<uint16_t>(value);
        result = static_cast<uint16_t>(0u - right);
        flags = subFlags16(0, right);
    }

    GP4_GLOBAL(uint8_t, 0x00604154) = static_cast<uint8_t>(result >> 8);
    r->edx = (r->edx & 0xffff0000u) | result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042ac9c, FUN_0042ac9c, "regs(edx:u32, ebx:ptr[0x128]:bytes, esi:ptr[0x98]:bytes) -> (edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
