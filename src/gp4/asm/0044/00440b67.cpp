#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// ZF, SF, PF of a 32-bit result.
static inline uint32_t resultFlags(uint32_t res) {
    return parityFlag(res) | (res == 0u ? 0x40u : 0u) | ((res & 0x80000000u) != 0u ? 0x80u : 0u);
}

// ZF, SF, PF of an 8-bit result (CF, OF, AF clear: a logic operation).
static inline uint32_t byteLogicFlags(uint8_t res) {
    return parityFlag(res) | (res == 0u ? 0x40u : 0u) | ((res & 0x80u) != 0u ? 0x80u : 0u);
}

// Flags of `shrd eax, edx, 14` on the 64-bit product edx:eax under the oracle model: CF = last bit out,
// AF = 0, OF = bit31(prev ^ result) with prev the product shifted by 13.
static inline uint32_t shrd14Flags(int64_t product) {
    const uint32_t result = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    const uint32_t prev = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 13);
    uint32_t flags = resultFlags(result);
    if ((prev & 1u) != 0u) flags |= 0x01u;
    if (((prev ^ result) & 0x80000000u) != 0u) flags |= 0x800u;
    return flags;
}

struct Unknown_Object {
    uint8_t Unknown00[0x73];
    uint8_t Index73;
    uint8_t Unknown74[8];
    uint8_t Flags7C;
    uint8_t Unknown7D[7];
    uint8_t Category84;
    uint8_t Unknown85[0x9e - 0x85];
    uint8_t Count9E;
    uint8_t Scale9F;
    uint8_t UnknownA0[0x161 - 0xa0];
    uint8_t Flags161;
};

// One byte of the generator 0x00401462 (al of its return value).
static inline uint32_t drawByte(gp4::Regs& c) {
    gp4::call_regs(0x00401462, c);
    return c.eax & 0xffu;
}

// Two generator bytes (first one high) as a 16-bit word.
static inline uint32_t drawWord(gp4::Regs& c) {
    const uint32_t high = drawByte(c);
    const uint32_t low = drawByte(c);
    return (high << 8) | low;
}

// pushal / popal: one random draw decides whether the object in esi gets a new target. The chance comes from
// the 11-word table at 0x0062ebd0 (index byte +0x73 - 1 & 0x1f, at most 10), scaled by [0x007ad8a2] when
// [0x007c2ea9] is 0x80, otherwise halved (quartered with bit 6). On a hit a second draw picks a category 0..7
// from the cumulative weights (words at 0x00627a8c plus [0x0062241c] each); an object with bit 7 of +0x7c set
// only accepts categories whose bit is set in [0x007ab540]. Categories 5 and 6 may turn into 8 and 9. Stores the
// category (+0x84), sets bit 2 of +0x161, a random count (+0x9e) and a scaled random byte (+0x9f). A miss clears
// bit 2 of +0x161. Registers are restored; the flags are those of the last flag-writing instruction: the
// byte `and` on a miss, the final `shrd` on a hit.
// FUNCTION: GP4 0x00440b67
void __cdecl FUN_00440b67(gp4::Regs* r) {
    Unknown_Object* object = reinterpret_cast<Unknown_Object*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    int32_t slot = static_cast<int32_t>((static_cast<uint32_t>(object->Index73) - 1u) & 0x1fu);
    if (slot > 10) slot = 10;
    uint32_t chance = GP4_ARRAY(const uint16_t, 0x0062ebd0)[slot];

    if (GP4_GLOBAL(uint8_t, 0x007c2ea9) == 0x80u) {
        uint32_t scale = GP4_GLOBAL(uint16_t, 0x007ad8a2);
        if (static_cast<int32_t>(scale) < 0x14) scale = 0x14u;
        chance = (scale * chance) / 100u;
    } else {
        chance >>= 1;
        if ((GP4_GLOBAL(uint8_t, 0x007c2ea9) & 0x40u) != 0u) chance >>= 1;
    }

    const int32_t roll = static_cast<int32_t>(drawWord(c) >> 2);
    bool accepted = roll < static_cast<int32_t>(chance);

    uint32_t category = 7u;
    if (accepted) {
        uint32_t total = 0u;
        for (uint32_t i = 0; i < 8u; ++i) {
            total += GP4_ARRAY(const uint16_t, 0x00627a8c)[i];
            total += GP4_GLOBAL(uint32_t, 0x0062241c);
        }
        const int64_t pick = static_cast<int64_t>(static_cast<int32_t>(drawWord(c) >> 2)) *
                             static_cast<int64_t>(static_cast<int32_t>(total));
        const int32_t target = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(pick) >> 14));

        uint32_t running = 0u;
        for (uint32_t i = 0; i < 8u; ++i) {
            running += GP4_ARRAY(const uint16_t, 0x00627a8c)[i];
            running += GP4_GLOBAL(uint32_t, 0x0062241c);
            if (target < static_cast<int32_t>(running)) {
                category = i;
                break;
            }
        }

        if ((object->Flags7C & 0x80u) != 0u && ((GP4_GLOBAL(uint32_t, 0x007ab540) >> category) & 1u) == 0u) {
            accepted = false;
        }
    }

    if (!accepted) {
        object->Flags161 = static_cast<uint8_t>(object->Flags161 & 0xfbu);
        r->eflags = (r->eflags & ~0x8d5u) | byteLogicFlags(object->Flags161);
        return;
    }

    if (category == 5u) {
        if (drawByte(c) >= 0x80u) category = 8u;
    } else if (category == 6u) {
        if (drawByte(c) >= 0x80u) category = 9u;
    }
    object->Category84 = static_cast<uint8_t>(category);
    object->Flags161 = static_cast<uint8_t>(object->Flags161 | 4u);

    int32_t bound = static_cast<int32_t>(GP4_GLOBAL(uint16_t, 0x007c2e96)) - 1;
    if (bound < 1) bound = 1;
    c.eax = static_cast<uint32_t>(bound);
    gp4::call_regs(0x00401495, c);
    object->Count9E = static_cast<uint8_t>(c.eax + 1u);

    const uint32_t scaled = ((drawByte(c)) + 1u) << 3;
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(scaled)) *
                            static_cast<int64_t>(GP4_GLOBAL(uint16_t, 0x00868558));
    object->Scale9F = static_cast<uint8_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14));

    r->eflags = (r->eflags & ~0x8d5u) | shrd14Flags(product);
}
GP4_IMPL(0x00440b67, FUN_00440b67, "regs(esi:ptr[0x169]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0066d8c0+carno_0066ff6c+weights_00627a8c")
