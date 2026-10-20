#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit TEST (CF = OF = AF = 0).
static inline uint32_t testFlags8(uint8_t result) {
    return byteParity(result)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80u) != 0u ? 0x080u : 0u);
}

struct Unknown_Car {
    uint8_t Unknown00[0x1c];
    uint32_t Position1C;
    uint8_t Unknown20[0x51];
    uint8_t Flags71;
    uint8_t Unknown72[0x0a];
    uint8_t Index7C;
};

// Counts one event for the car's slot (clamped to 21, 99 maximum) unless the car is past the pit start
// and has bit 1 of +0x71 set, then calls 0x0040f0dd for the same car.
// FUNCTION: GP4 0x00442c41
void __cdecl FUN_00442c41(gp4::Regs* r) {
    const Unknown_Car* car = reinterpret_cast<const Unknown_Car*>(static_cast<uintptr_t>(r->esi));
    const uint32_t position = car->Position1C;
    r->eax = position;

    if (position >= GP4_GLOBAL(uint32_t, 0x00891500)) {
        const uint8_t masked = static_cast<uint8_t>(car->Flags71 & 0x02u);
        r->eflags = (r->eflags & ~0x8d5u) | testFlags8(masked);
        if (masked != 0u) {
            return;
        }
    }

    int32_t slot = static_cast<int32_t>(car->Index7C & 0x3fu) - 1;
    if (slot > 0x15) {
        slot = 0x15;
    }
    uint32_t count = static_cast<uint32_t>(GP4_ARRAY(uint8_t, 0x0066ec4a)[slot]) + 1u;
    if (static_cast<int32_t>(count) > 0x63) {
        count = 0x63;
    }
    GP4_ARRAY(uint8_t, 0x0066ec4a)[slot] = static_cast<uint8_t>(count);

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = static_cast<uint32_t>(slot);
    call.edx = count;
    gp4::call_regs(0x0040f0dd, call);

    r->eax = static_cast<uint32_t>(slot);
    r->edx = count;
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x00442c41, FUN_00442c41, "regs(esi:ptr[0x84]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=counter_0040f0dd+counts_0066ec49")
