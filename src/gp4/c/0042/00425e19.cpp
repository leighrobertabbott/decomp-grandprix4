#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Record addressed through esi (only the fields this routine reads).
struct Unknown_00425e19_Esi {
    uint8_t Unknown00[0x78];
    uint8_t Flags78;
    uint8_t Unknown79[0xa0 - 0x79];
    int16_t UnknownA0;
    uint8_t UnknownA2[0xd6 - 0xa2];
    uint16_t UnknownD6;
    uint8_t UnknownD8[0xf6 - 0xd8];
    int16_t UnknownF6;
    uint8_t UnknownF8[0x122 - 0xf8];
    uint8_t Flags122;
};

// imul (signed 64-bit product) followed by shrd eax, edx, count; keeps the last product.
struct MulShr {
    int64_t product;
    static inline uint32_t apply(MulShr& m, int32_t a, int32_t b, unsigned count) {
        m.product = (int64_t)a * (int64_t)b;
        return (uint32_t)((uint64_t)m.product >> count);
    }
};

// FUNCTION: GP4 0x00425e19
// A chain of fixed-point products: (table[edi] * word(esi+0xd6) >> 14) * DAT_00603854 >> 2, then
// either (for a negative DAT_00603d40 - DAT_00624790) times ((-diff) * DAT_00624798 >> 14) >> 14
// using table 0x0063acd4, or (diff >= 0) times (word(esi+0xf6) >> 14) times
// (diff * DAT_00624794 >> 14), optionally scaled by DAT_006039d8 (byte esi+0x78 bit 6) and
// DAT_00604164 (byte esi+0x122 bit 4) and finally by word(esi+0xa0), using table 0x00639654.
// ecx is preserved; edx leaves holding the high half of the last imul and the flags are
// those of the final shrd.
void __cdecl FUN_00425e19(gp4::Regs* r) {
    const Unknown_00425e19_Esi* rec = reinterpret_cast<const Unknown_00425e19_Esi*>(static_cast<uintptr_t>(r->esi));
    const uint32_t index = r->edi;
    MulShr m;

    const uint32_t diff = GP4_GLOBAL(uint32_t, 0x00603d40) - GP4_GLOBAL(uint32_t, 0x00624790);
    uint32_t eax;
    if ((diff & 0x80000000u) != 0u) {
        const uint32_t scale = MulShr::apply(m, (int32_t)(0u - diff), GP4_GLOBAL(int32_t, 0x00624798), 14);
        eax = MulShr::apply(m, GP4_ARRAY(int32_t, 0x0063acd4)[index], (int32_t)rec->UnknownD6, 14);
        eax = MulShr::apply(m, (int32_t)eax, GP4_GLOBAL(int32_t, 0x00603854), 2);
        eax = MulShr::apply(m, (int32_t)eax, (int32_t)scale, 14);
    } else {
        const uint32_t scale = MulShr::apply(m, (int32_t)diff, GP4_GLOBAL(int32_t, 0x00624794), 14);
        eax = MulShr::apply(m, GP4_ARRAY(int32_t, 0x00639654)[index], (int32_t)rec->UnknownD6, 14);
        eax = MulShr::apply(m, (int32_t)eax, GP4_GLOBAL(int32_t, 0x00603854), 2);
        eax = MulShr::apply(m, (int32_t)eax, (int32_t)rec->UnknownF6, 14);
        eax = MulShr::apply(m, (int32_t)eax, (int32_t)scale, 14);
        if ((rec->Flags78 & 0x40u) != 0u) {
            eax = MulShr::apply(m, (int32_t)eax, GP4_GLOBAL(int32_t, 0x006039d8), 14);
        }
        if ((rec->Flags122 & 0x10u) != 0u) {
            eax = MulShr::apply(m, (int32_t)eax, GP4_GLOBAL(int32_t, 0x00604164), 14);
        }
        eax = MulShr::apply(m, (int32_t)eax, (int32_t)rec->UnknownA0, 14);
    }

    const uint32_t low = (uint32_t)m.product;
    const uint32_t high = (uint32_t)((uint64_t)m.product >> 32);
    const uint32_t previous = (uint32_t)((uint64_t)m.product >> 13);
    // shrd by 14: CF = last bit shifted out; OF = bit31(previous ^ result), AF = 0 (pinned oracle model)
    const uint32_t flags = ((low >> 13) & 1u)
        | parityFlag(eax)
        | (eax == 0u ? 0x40u : 0u)
        | ((eax & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((previous ^ eax) >> 31) & 1u) != 0u ? 0x800u : 0u);

    r->eax = eax;
    r->edx = high;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00425e19, FUN_00425e19, "regs(esi:ptr[0x130]:bytes, edi:u32[0..21]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
