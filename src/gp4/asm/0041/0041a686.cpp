#include <gp4/gp4.h>

// imul (one operand) followed by shrd eax, edx, N: low 32 bits of the 64-bit product >> N
template <int N>
static inline uint32_t mulShift(int32_t a, int32_t b) {
    return static_cast<uint32_t>(static_cast<uint64_t>(static_cast<int64_t>(a) * b) >> N);
}

// |value| as the original computes it (test / jns / neg)
static inline uint32_t absWord(uint32_t value) {
    return (static_cast<int32_t>(value) < 0) ? 0u - value : value;
}

// FUNCTION: GP4 0x0041a686
void __cdecl FUN_0041a686(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(r->esi);

    if ((GP4_FIELD(uint8_t, body, 0xe5) & 4u) != 0u) {
        // test byte [esi+0xe5], 4 -> nonzero: ZF = PF = SF = CF = OF = AF = 0; nothing else changes
        r->eflags = r->eflags & ~0x8d5u;
        return;
    }

    uint8_t* record = GP4_ARRAY(uint8_t, 0x006380d8);
    uint32_t index = 0u;
    uint32_t lastLow = 0u;
    uint32_t lastHigh = 0u;
    do {
        const uint32_t spread = absWord(GP4_FIELD(uint32_t, record, 0x40)) + absWord(GP4_FIELD(uint32_t, record, 0x18));
        const uint32_t scaled = mulShift<0x12>(static_cast<int32_t>(spread),
                                               GP4_FIELD(int32_t, body, 0x2ec + index * 4u));
        uint32_t weighted = mulShift<0xe>(static_cast<int32_t>(scaled),
                                          GP4_ARRAY(int32_t, 0x00601dd4)[index]);
        if (static_cast<int32_t>(weighted) < 0) {
            weighted = 0u;
        }
        const int64_t product = static_cast<int64_t>(static_cast<int32_t>(weighted)) * GP4_GLOBAL(int32_t, 0x0060384c);
        lastLow = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 12);
        lastHigh = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
        GP4_FIELD(uint32_t, body, 0x188 + index * 4u) += lastLow;
        ++index;
        record += 0x300;
    } while (record != GP4_ARRAY(uint8_t, 0x00638cd8));

    // the loop exits on cmp ebx, 0x638cd8 -> equal: ZF = PF = 1, the rest clear
    r->eax = lastLow;
    r->ebx = 0x00638cd8u;
    r->ecx = index;
    r->edx = lastHigh;
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0041a686, FUN_0041a686, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:ptr[0x300]:bytes) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=wheels_0041a686")
