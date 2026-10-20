#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of `left + right` whose result is `result`.
static inline uint32_t addFlags(uint32_t left, uint32_t right, uint32_t result) {
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

// Entry of the 16-byte table at 0x868598: dword +0 and dword +0xc are the two interpolated values.
struct Unknown_00868598 {
    int32_t Unknown00;
    uint8_t Unknown04[8];
    int32_t Unknown0C;
};

// Linear interpolation (14-bit fraction in eax) between two table entries named by two words of
// the 0x160-byte record at edi (+0xaa/+0x20a when edx == 3, else +0x132/+0x292):
//   edx = entry[b].+0xc interpolated from entry[a].+0xc, eax = the same for dword +0.
// edi, esi and ebp are saved and restored; the flags are those of the final "add eax, [esi+...]".
// FUNCTION: GP4 0x00403189
void __cdecl FUN_00403189(gp4::Regs* r) {
    const int32_t fraction = static_cast<int32_t>(r->eax);
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    uint32_t from;
    uint32_t to;
    if (r->edx == 3u) {
        from = GP4_FIELD(uint16_t, record, 0xaa);
        to = GP4_FIELD(uint16_t, record, 0x20a);
    } else {
        from = GP4_FIELD(uint16_t, record, 0x132);
        to = GP4_FIELD(uint16_t, record, 0x292);
    }
    const Unknown_00868598* table = GP4_ARRAY(Unknown_00868598, 0x00868598);
    const Unknown_00868598* start = table + from;
    const Unknown_00868598* end = table + to;

    const int64_t productC = static_cast<int64_t>(static_cast<int32_t>(static_cast<uint32_t>(end->Unknown0C) - static_cast<uint32_t>(start->Unknown0C))) * fraction;
    const uint32_t valueC = (static_cast<uint32_t>(static_cast<uint64_t>(productC) >> 14)) + static_cast<uint32_t>(start->Unknown0C);

    const int64_t product0 = static_cast<int64_t>(static_cast<int32_t>(static_cast<uint32_t>(end->Unknown00) - static_cast<uint32_t>(start->Unknown00))) * fraction;
    const uint32_t shifted0 = static_cast<uint32_t>(static_cast<uint64_t>(product0) >> 14);
    const uint32_t value0 = shifted0 + static_cast<uint32_t>(start->Unknown00);

    r->eax = value0;
    r->edx = valueC;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(shifted0, static_cast<uint32_t>(start->Unknown00), value0);
}
GP4_IMPL(0x00403189, FUN_00403189, "regs(eax:i32, edx:u32[2..4], edi:ptr[0x2a0]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
