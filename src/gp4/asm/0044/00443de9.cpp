#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of `shl value, 3` under the oracle model: CF = last bit shifted out, AF = 0,
// OF = bit31(value << 2 ^ result).
static inline uint32_t shl3Flags32(uint32_t value) {
    const uint32_t result = value << 3;
    return ((value >> 29) & 1u)
        | byteParity(result)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((((value << 2) ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_Object {
    uint8_t Unknown00[0x18];
    int32_t X18;
    uint8_t Unknown1C[4];
    int32_t Z20;
    uint8_t Unknown24[0x60];
    uint32_t Record84;
    int32_t Value88;
};

// Seeds the point (0x00623e30, 0x00623e38) from the object's position (>> 6), lets 0x004061ed walk the
// record ring from the object's current record and stores the record reached plus its word +6 scaled by 8.
// FUNCTION: GP4 0x00443de9
void __cdecl FUN_00443de9(gp4::Regs* r) {
    Unknown_Object* object = reinterpret_cast<Unknown_Object*>(static_cast<uintptr_t>(r->esi));
    const uint32_t record = object->Record84;

    GP4_GLOBAL(int32_t, 0x00623e30) = object->X18 >> 6;
    GP4_GLOBAL(int32_t, 0x00623e38) = object->Z20 >> 6;

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.edi = record;
    gp4::call_regs(0x004061ed, call);

    object->Record84 = call.edx;
    const int16_t word = *reinterpret_cast<const int16_t*>(static_cast<uintptr_t>(object->Record84) + 6u);
    const int32_t widened = word;
    object->Value88 = static_cast<int32_t>(static_cast<uint32_t>(widened) << 3);

    // pushal/popal restore every register; the flags are those of the final shl.
    r->eflags = (r->eflags & ~0x8d5u) | shl3Flags32(static_cast<uint32_t>(widened));
}
GP4_IMPL(0x00443de9, FUN_00443de9, "regs(esi:ptr[0x90]:layout=view_00443de9) -> (cf, pf, af, zf, sf, of); globals=layout=ring_00405a12+w6_008943a0")
