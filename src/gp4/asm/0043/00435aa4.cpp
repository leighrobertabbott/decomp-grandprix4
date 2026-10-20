#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of "add reg32, reg32": CF, PF, AF, ZF, SF, OF
static inline uint32_t addFlags(uint32_t a, uint32_t b, uint32_t sum) {
    return (sum < a ? 0x001u : 0u)
        | parityFlag(sum)
        | (((a ^ b ^ sum) & 0x10u) ? 0x010u : 0u)
        | (sum == 0 ? 0x040u : 0u)
        | ((sum & 0x80000000u) ? 0x080u : 0u)
        | ((((a ^ sum) & (b ^ sum)) & 0x80000000u) ? 0x800u : 0u);
}

// (value * 0x00603858) >> 14, taken as the low dword of the 64-bit product (shrd eax, edx, 14),
// limited to 0x4000.
static inline int32_t blendFactor(int32_t value) {
    const int64_t product = static_cast<int64_t>(value) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603858));
    const int32_t factor = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14));
    return factor > 0x4000 ? 0x4000 : factor;
}

// FUNCTION: GP4 0x00435aa4
// Q14 blend of the dwords at +0x288 and +0x28c toward the dword at +0xac:
// dst = (+0xac * t >> 14) + ((0x4000 - t) * dst >> 14), with t = min(factor * DAT_00603858 >> 14, 0x4000).
// The first factor is DAT_006033e8 when +0xac and +0x288 share a sign and |+0xac| > |+0x288|, else
// DAT_006033ec; the second is DAT_006033f8. Leaves ecx = t, ebp = first term, eax = the sum,
// edx = high dword of the last product and the flags of the final add.
void __cdecl FUN_00435aa4(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    const int32_t source = GP4_FIELD(int32_t, body, 0xac);
    int32_t first = source;
    int32_t second = GP4_FIELD(int32_t, body, 0x288);

    int32_t selected = GP4_GLOBAL(int32_t, 0x006033ec);
    if (((static_cast<uint32_t>(first) ^ static_cast<uint32_t>(second)) & 0x80000000u) == 0u) {
        if (first < 0) {
            first = static_cast<int32_t>(0u - static_cast<uint32_t>(first));
        }
        if (second < 0) {
            second = static_cast<int32_t>(0u - static_cast<uint32_t>(second));
        }
        if (first > second) {
            selected = GP4_GLOBAL(int32_t, 0x006033e8);
        }
    }

    int32_t t = blendFactor(selected);
    int64_t moved = static_cast<int64_t>(source) * static_cast<int64_t>(t);
    uint32_t termA = static_cast<uint32_t>(static_cast<uint64_t>(moved) >> 14);
    int64_t kept = static_cast<int64_t>(0x4000 - t) * static_cast<int64_t>(GP4_FIELD(int32_t, body, 0x288));
    uint32_t termB = static_cast<uint32_t>(static_cast<uint64_t>(kept) >> 14);
    GP4_FIELD(uint32_t, body, 0x288) = termB + termA;

    t = blendFactor(GP4_GLOBAL(int32_t, 0x006033f8));
    moved = static_cast<int64_t>(GP4_FIELD(int32_t, body, 0xac)) * static_cast<int64_t>(t);
    termA = static_cast<uint32_t>(static_cast<uint64_t>(moved) >> 14);
    kept = static_cast<int64_t>(0x4000 - t) * static_cast<int64_t>(GP4_FIELD(int32_t, body, 0x28c));
    termB = static_cast<uint32_t>(static_cast<uint64_t>(kept) >> 14);
    const uint32_t sum = termB + termA;
    GP4_FIELD(uint32_t, body, 0x28c) = sum;

    r->eax = sum;
    r->ecx = static_cast<uint32_t>(t);
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(kept) >> 32);
    r->ebp = termA;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(termB, termA, sum);
}
GP4_IMPL(0x00435aa4, FUN_00435aa4, "regs(esi:ptr[0x294]:bytes) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
