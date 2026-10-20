#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit logic result (and): CF = OF = AF = 0.
static inline uint32_t logicFlags(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// An 8-byte entry of the table at 0x00604435 (the entries are read unaligned): a word at +0 shifted
// left by 14 plus the signed word at +2.
static inline uint32_t entryValue(uint32_t address) {
    uint8_t* entry = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(address));
    const uint32_t high = GP4_FIELD(uint16_t, entry, 0);
    const int32_t low = static_cast<int32_t>(GP4_FIELD(int16_t, entry, 2));
    return (high << 14) + static_cast<uint32_t>(low);
}

// Interpolates between the table value of the segment record at edi and that of the next record
// (or, at the pit end [0x00866ef8], the same record plus 0x4000) by the signed word [esi+0x66]:
// returns the 14-bit fraction in eax and the whole part in edx.
// FUNCTION: GP4 0x0040daeb
void __cdecl FUN_0040daeb(gp4::Regs* r) {
    const uint32_t record = r->edi;
    void* current = reinterpret_cast<void*>(static_cast<uintptr_t>(record));
    const uint32_t startAddress = 0x00604435u + static_cast<uint32_t>(GP4_FIELD(uint16_t, current, 0x7a)) * 8u;
    const uint32_t start = entryValue(startAddress);

    uint32_t target;
    uint32_t endAddress;
    uint32_t endRecord;
    const uint32_t next = record + 0x160u;
    if (next < GP4_GLOBAL(uint32_t, 0x00866ef8)) {
        endRecord = next;
        void* nextRecord = reinterpret_cast<void*>(static_cast<uintptr_t>(next));
        endAddress = 0x00604435u + static_cast<uint32_t>(GP4_FIELD(uint16_t, nextRecord, 0x7a)) * 8u;
        target = entryValue(endAddress);
    } else {
        endRecord = record;
        endAddress = 0x00604435u + static_cast<uint32_t>(GP4_FIELD(uint16_t, current, 0x7a)) * 8u;
        target = entryValue(endAddress) + 0x4000u;
    }

    void* source = reinterpret_cast<void*>(static_cast<uintptr_t>(r->esi));
    const int32_t fraction = static_cast<int32_t>(GP4_FIELD(int16_t, source, 0x66));
    const int32_t difference = static_cast<int32_t>(target - start);
    const int64_t product = static_cast<int64_t>(difference) * static_cast<int64_t>(fraction);
    const uint32_t interpolated = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14) + start;

    r->ebx = start;
    r->ecx = endAddress;
    r->ebp = static_cast<uint32_t>(fraction);
    r->edi = endRecord;
    r->edx = interpolated >> 14;
    r->eax = interpolated & 0x3fffu;
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags(r->eax);
}
GP4_IMPL(0x0040daeb, FUN_0040daeb, "regs(esi:ptr[0x70]:bytes, edi:rec[0x8943a0,0x160,0..24]) -> (eax, ebx, ecx, edx, ebp, edi, cf, pf, af, zf, sf, of); globals=layout=lanes_0040da62+pend_0040daeb")
