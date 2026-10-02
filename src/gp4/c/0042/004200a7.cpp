#include <gp4/gp4.h>

// Four 0x300-byte records at 0x006380d8 (only the fields this routine touches).
struct Unknown_006380d8 {
    uint8_t Unknown000[0x6c];
    int32_t Unknown06c;
    uint8_t Unknown070[0xb4 - 0x70];
    uint32_t Unknown0b4;
    uint8_t Unknown0b8[0x15c - 0xb8];
    int32_t Unknown15c;
    int32_t Unknown160;
    uint32_t Unknown164;
    uint8_t Unknown168[0x2c4 - 0x168];
    int32_t Unknown2c4;
    uint8_t Unknown2c8[0x300 - 0x2c8];
};
static_assert(sizeof(Unknown_006380d8) == 0x300, "record stride");

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x004200a7
void __cdecl FUN_004200a7(gp4::Regs* r) {
    uint32_t* owner = (uint32_t*)(r->esi + 0x37c);   // eight i32 high words at +0x37c..+0x398
    Unknown_006380d8* const records = GP4_ARRAY(Unknown_006380d8, 0x006380d8);
    const int32_t scale = GP4_GLOBAL(int32_t, 0x006228ac);
    uint32_t edx = r->edx;
    uint32_t ebp = r->ebp;

    if (GP4_GLOBAL(uint32_t, 0x00601d38) != 0) {
        Unknown_006380d8* rec = records;
        uint32_t* high = owner;
        for (int k = 4; k != 0; k--, rec++, high++) {
            // 64-bit accumulator: low word in the record, high word in the owner.
            const int64_t p = (int64_t)rec->Unknown06c * (int64_t)scale;
            const uint32_t lo = (uint32_t)p << 16;
            const uint32_t before = rec->Unknown0b4;
            rec->Unknown0b4 = before + lo;
            *high += (uint32_t)((uint64_t)p >> 16) + ((rec->Unknown0b4 < before) ? 1u : 0u);
        }
    }

    {
        Unknown_006380d8* rec = records;
        uint32_t* high = owner + 4;
        for (int k = 4; k != 0; k--, rec++, high++) {
            const int32_t old = rec->Unknown15c;
            const int64_t rate = (int64_t)rec->Unknown160 * (int64_t)scale;
            rec->Unknown15c = (int32_t)((uint32_t)old + (uint32_t)((uint64_t)rate >> 16));
            const int32_t mid = (int32_t)((uint32_t)((int32_t)((uint32_t)rec->Unknown15c - (uint32_t)old) >> 1)
                                          + (uint32_t)old);
            const int64_t p = (int64_t)mid * (int64_t)scale;
            const uint32_t lo = (uint32_t)p << 12;
            edx = (uint32_t)((uint64_t)p >> 20);
            const uint32_t before = rec->Unknown164;
            rec->Unknown164 = before + lo;
            *high += edx + ((rec->Unknown164 < before) ? 1u : 0u);
            ebp = (uint32_t)old;
        }
    }

    uint32_t a = 0, b = 0, sum = 0;
    {
        Unknown_006380d8* rec = records;
        const uint32_t* step = GP4_ARRAY(const uint32_t, 0x0062244c);
        for (int k = 2; k != 0; k--, rec += 2, step++) {
            a = (uint32_t)rec[0].Unknown2c4;
            b = *step;
            sum = a + b;
            rec[0].Unknown2c4 = (int32_t)sum;
            rec[1].Unknown2c4 = (int32_t)sum;
        }
    }

    const uint32_t flags = (sum < a ? 1u : 0u)
        | parityFlag(sum)
        | ((a ^ b ^ sum) & 0x10u)
        | (sum == 0u ? 0x40u : 0u)
        | ((sum & 0x80000000u) ? 0x80u : 0u)
        | (((~(a ^ b) & (a ^ sum)) & 0x80000000u) ? 0x800u : 0u);

    r->eax = sum;
    r->ebx = 0x006380d8u;
    r->edx = edx;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004200a7, FUN_004200a7, "regs(esi:ptr[0x3a4]:bytes) -> (eax, ebx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
