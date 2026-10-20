#include <gp4/gp4.h>

static inline uint32_t ParityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x4u : 0u;
}

// Chooses the 0..0x4000 factor stored at 0x638d50 (may update 0x638d34 and +0x1ce).
static inline int32_t SelectFactor(uint8_t* rec) {
    if (GP4_GLOBAL(uint8_t, 0x00622424) != 0) return 0x4000;
    if (rec[0x72] == 0) return 0;
    if ((rec[0xfe] & 0x20) == 0) return 0;
    if ((rec[0x8d] & 0x10) != 0 && rec[0x84] == 4) return 0;
    if (GP4_FIELD(uint16_t, rec, 0x1ae) == 0 && (rec[0x6f] & 0x10) != 0) {
        if ((rec[0xff] & 0x04) == 0) return 0;
        GP4_GLOBAL(int32_t, 0x00638d34) = 0;
    }

    const int32_t level = GP4_GLOBAL(int32_t, 0x00623c34) >> 8;
    const int32_t cap = GP4_GLOBAL(int32_t, 0x0062fb78);

    if ((rec[0xe6] & 0x01) == 0 && GP4_GLOBAL(int32_t, 0x006258bc) == 0) {
        if (GP4_GLOBAL(uint8_t, 0x0062fb4a) != 0) {
            if ((rec[0x1ce] & 0x08) == 0) {
                if (level >= GP4_GLOBAL(int32_t, 0x0062fb88)) return cap;
                rec[0x1ce] |= 0x08;
            } else if (level >= GP4_GLOBAL(int32_t, 0x0062fb84)) {
                rec[0x1ce] &= 0xf7;
                return cap;
            }
            return (rec[0x21c] & 0x20) != 0 ? cap : 0;
        }

        const int32_t w = static_cast<int32_t>(GP4_FIELD(int16_t, rec, 0x9a));
        int32_t value = 0;
        bool check;
        if (w >= GP4_GLOBAL(int32_t, 0x00624760)) {
            check = level >= GP4_GLOBAL(int32_t, 0x0062476c);
            if (!check) value = GP4_GLOBAL(int32_t, 0x00624754);
        } else {
            check = level >= GP4_GLOBAL(int32_t, 0x00624770);
            if (!check) {
                value = (rec[0x21c] & 0x20) != 0 ? GP4_GLOBAL(int32_t, 0x0062475c)
                                                  : GP4_GLOBAL(int32_t, 0x00624758);
            }
        }
        if (check) {
            const int32_t ref = GP4_GLOBAL(int32_t, 0x00638dc8);
            uint32_t dist = static_cast<uint32_t>(level) - static_cast<uint32_t>(ref);
            if (static_cast<int32_t>(dist) < 0) dist = 0u - dist;
            if (static_cast<int32_t>(dist) > GP4_GLOBAL(int32_t, 0x00624768)) {
                value = level >= ref ? GP4_GLOBAL(int32_t, 0x00624764)
                                     : GP4_GLOBAL(int32_t, 0x00624754);
            } else {
                value = 0;
            }
        }
        GP4_GLOBAL(int32_t, 0x00638d34) = value;
    }

    const int32_t target = GP4_GLOBAL(int32_t, 0x00638d34);
    if (target <= 0x1000) return 0x4000;
    int32_t clamped = target;
    if (clamped < 0x1000) clamped = 0x1000;
    if (clamped > 0x3000) clamped = 0x3000;
    int32_t factor = 0x4000 - (clamped - 0x1000) * 2;
    if (GP4_GLOBAL(uint8_t, 0x0062fb4a) != 0 &&
        level >= GP4_GLOBAL(int32_t, 0x0062fb88) && factor > cap) {
        factor = cap;
    }
    return factor;
}

// FUNCTION: GP4 0x0041b950
void __cdecl FUN_0041b950(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const int32_t factor = SelectFactor(rec);
    GP4_GLOBAL(int32_t, 0x00638d50) = factor;

    // imul; shrd eax, edx, 14
    const int64_t product = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006258b8)) * factor;
    const uint64_t bits = static_cast<uint64_t>(product);
    const uint32_t result = static_cast<uint32_t>(bits >> 14);
    const uint32_t previous = static_cast<uint32_t>(bits >> 13);
    GP4_GLOBAL(uint32_t, 0x006258b4) = result;
    r->eax = result;
    r->edx = static_cast<uint32_t>(bits >> 32);

    // shrd flags (pinned Unicorn model): CF = last bit out, AF = 0,
    // OF = bit31(previous ^ result).
    const uint32_t flags = (previous & 1u)
        | ParityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((previous ^ result) & 0x80000000u) != 0u) ? 0x800u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041b950, FUN_0041b950, "regs(esi:ptr[0x224]:layout=rec_0041b950) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=state_0041b950")
