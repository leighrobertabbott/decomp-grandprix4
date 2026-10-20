#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of `sar value, 5` under the oracle model: CF = last bit shifted out, AF = OF = 0,
// ZF/SF/PF from the result.
static inline uint32_t sar5Flags32(int32_t value) {
    const int32_t result = value >> 5;
    const uint32_t bits = static_cast<uint32_t>(result);
    return ((static_cast<uint32_t>(value) >> 4) & 1u)
        | parityFlag(bits)
        | (bits == 0u ? 0x40u : 0u)
        | ((bits & 0x80000000u) != 0u ? 0x80u : 0u);
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

// Re-seats the object on the track: clears the pit-search flags 0x627944/45/47, lets 0x00406dbb settle
// the object's position (x, z << 5) from its current record, stores the record reached, then lets
// 0x00407811 derive a value from the result and stores it >> 5 at +0x88. pushal/popal restore every
// register; the flags are those of the final sar.
// FUNCTION: GP4 0x00443e26
void __cdecl FUN_00443e26(gp4::Regs* r) {
    Unknown_Object* object = reinterpret_cast<Unknown_Object*>(static_cast<uintptr_t>(r->esi));

    GP4_GLOBAL(uint8_t, 0x00627945) = 0;
    GP4_GLOBAL(uint8_t, 0x00627944) = 0;
    GP4_GLOBAL(uint8_t, 0x00627947) = 0;

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.edi = object->Record84;
    call.eax = static_cast<uint32_t>(object->X18) << 5;
    call.edx = static_cast<uint32_t>(object->Z20) << 5;
    gp4::call_regs(0x00406dbb, call);

    object->Record84 = call.edi;
    gp4::call_regs(0x00407811, call);

    const int32_t value = static_cast<int32_t>(call.eax);
    object->Value88 = value >> 5;

    r->eflags = (r->eflags & ~0x8d5u) | sar5Flags32(value);
}
GP4_IMPL(0x00443e26, FUN_00443e26, "regs(esi:ptr[0x90]:layout=view_00443de9) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trackgeo_008943a0+tbl_00868598+state_0040705f+state_00407af6+state_0040759e+trackfar_008943a0+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+pit_00406dbb+segflag_004381ab")
