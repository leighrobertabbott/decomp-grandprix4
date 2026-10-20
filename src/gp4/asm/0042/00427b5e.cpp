#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// imul + shrd eax, edx, shift: the low dword of the 64-bit product shifted right.
static inline uint32_t mulShift(int32_t a, int32_t b, int shift) {
    const int64_t product = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> shift);
}

// FUNCTION: GP4 0x00427b5e
// esi = record, edi = block whose word at +2 feeds [0x603d48]. Clears bit 5 of
// [esi+0x230]; [0x603d48] = ((sword [edi+2] * [0x622481]) >> 14) * [0x603854] >> 2.
// ecx = ([esi+0x288] * [0x603854]) >> 18: zero stores [0x6247a8] into [0x6247b8]
// and returns (eax = [0x6247a8], ecx = 0, edx = high dword of the imul, flags of
// "or ecx, ecx"); otherwise |ecx| goes through 0x0042790f (whose eax is added to
// [0x603d48]) and then 0x00427880, whose registers and flags are the result.
void __cdecl FUN_00427b5e(gp4::Regs* r) {
    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t* const block = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->edi));
    const int32_t rate = GP4_GLOBAL(int32_t, 0x00603854);

    GP4_FIELD(uint8_t, rec, 0x230) &= 0xdfu;

    // movsx eax, word [edi+2]; imul [0x622481]; shrd 14; (push/pop edx); imul [0x603854]; shrd 2
    const uint32_t scaled = mulShift(static_cast<int32_t>(GP4_FIELD(int16_t, const_cast<uint8_t*>(block), 2)),
                                     GP4_GLOBAL(int32_t, 0x00622481), 0xe);
    GP4_GLOBAL(uint32_t, 0x00603d48) = mulShift(static_cast<int32_t>(scaled), rate, 2);

    // mov eax, [esi+0x288]; imul [0x603854]; shrd 18
    const int64_t product = static_cast<int64_t>(GP4_FIELD(int32_t, rec, 0x288)) * rate;
    uint32_t ecx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 18);
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);

    if (ecx == 0u) {
        const uint32_t value = GP4_GLOBAL(uint32_t, 0x006247a8);
        GP4_GLOBAL(uint32_t, 0x006247b8) = value;
        r->eax = value;
        r->ecx = 0u;
        r->edx = high;
        // or ecx, ecx on zero: ZF and PF set, CF/OF/SF/AF clear
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }

    if (static_cast<int32_t>(ecx) < 0) ecx = 0u - ecx;

    gp4::Regs sub = *r;
    sub.ecx = ecx;
    sub.edx = high;
    gp4::call_regs(0x0042790f, sub);
    // add [0x603d48], eax (its flags are replaced by 0x00427880)
    GP4_GLOBAL(uint32_t, 0x00603d48) += sub.eax;
    gp4::call_regs(0x00427880, sub);

    r->eax = sub.eax;
    r->ecx = sub.ecx;
    r->edx = sub.edx;
    r->ebp = sub.ebp;
    r->eflags = sub.eflags;
}
GP4_IMPL(0x00427b5e, FUN_00427b5e, "regs(edi:ptr[0x40]:bytes, esi:ptr[0x290]:i32[-64..64]) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00427880")
