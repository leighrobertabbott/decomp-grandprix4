#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 8-bit `cmp left, right`.
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u);
}

// Flags of `shrd eax, edx, 14` after a 64-bit product (pinned oracle model: CF = last bit out,
// AF = 0, OF = bit 31 of (value shifted by 13) xor (value shifted by 14)).
static inline uint32_t shrd14Flags(uint64_t product) {
    const uint32_t result = static_cast<uint32_t>(product >> 14);
    const uint32_t previous = static_cast<uint32_t>(product >> 13);
    return parityFlag8(result)
        | (((product >> 13) & 1u) != 0u ? 0x01u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((previous ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

struct Unknown_0043ced2_Car {
    uint8_t Unknown00[0x7c];
    uint8_t Unknown7C;
};

static inline uint32_t wordAt(uint32_t base, uint32_t offset) {
    return *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(base + offset));
}

// Low byte returned by the 0x00401462 generator.
static inline uint32_t nextRandomByte(const gp4::Regs* r) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x00401462, call);
    return call.eax & 0xffu;
}

// `value` scaled around 0x4000 by DAT_00622410; the high half of the product is left in edx.
static inline uint32_t scaleAround4000(uint32_t value, uint32_t* edx) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(value - 0x4000u))
        * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00622410));
    *edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14) + 0x4000u;
}

// FUNCTION: GP4 0x0043ced2
// esi = car record whose byte +0x7c is the car number (bit 7 = flag, low 6 bits = index + 1).
// Returns eax = a Q14 factor: DAT_0062eca4 when the per-car byte table 0x007c2a74 says 2, else a
// random/difficulty scaled value (by DAT_007ad8a0) multiplied through the 0x0043cdfe table value
// unless the car is flagged, in which case the table value or 0x4000 is returned. edx and the flags
// come from the last writer; ebx and ecx are restored.
void __cdecl FUN_0043ced2(gp4::Regs* r) {
    const Unknown_0043ced2_Car* car = reinterpret_cast<const Unknown_0043ced2_Car*>(static_cast<uintptr_t>(r->esi));
    const uint8_t carByte = car->Unknown7C;
    const uint32_t index = (carByte & 0x3fu) - 1u;

    const uint8_t special = GP4_ARRAY(uint8_t, 0x007c2a74)[static_cast<int32_t>(index)];
    if (special == 2u) {
        r->eax = GP4_GLOBAL(uint32_t, 0x0062eca4);
        r->eflags = (r->eflags & ~0x8d5u) | cmpFlags8(special, 2u);
        return;
    }

    uint32_t ebx = 0;
    if (GP4_GLOBAL(int8_t, 0x007c2ea9) < 0) {
        ebx = 2;
    }

    const int16_t mode = GP4_GLOBAL(int16_t, 0x007ad8a0);
    uint32_t edx;
    uint32_t value;
    if (mode > 1) {
        const uint32_t rnd = nextRandomByte(r);
        const int64_t product = static_cast<int64_t>(static_cast<int32_t>(rnd)) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0062240c));
        value = 0x4000u - static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
        value = scaleAround4000(value, &edx);
    } else if (mode < 1) {
        value = scaleAround4000(0x4000u, &edx);
    } else {
        ebx += index * 4u;
        const uint32_t high = nextRandomByte(r);
        const uint32_t low = nextRandomByte(r);
        const uint32_t noise = ((high << 8) + low) & 0x3fffu;
        int64_t product = static_cast<int64_t>(static_cast<int32_t>(noise)) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00622414));
        value = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
        product = static_cast<int64_t>(static_cast<int32_t>(value)) * static_cast<int64_t>(wordAt(0x0062eb78, ebx));
        value = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
        value += static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(wordAt(0x0066d7aa, index * 2u))));
        edx = wordAt(0x0062eb20, ebx);
        value += edx;
        value += GP4_GLOBAL(uint32_t, 0x00622418);
    }

    edx = (edx & 0xffffff00u) | carByte;
    uint32_t result;
    uint32_t flags;
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = GP4_GLOBAL(uint16_t, 0x007ad89e);
    if (static_cast<int8_t>(carByte) < 0) {
        const uint8_t selector = GP4_GLOBAL(uint8_t, 0x007ab020);
        bool fixed = false;
        flags = cmpFlags8(selector, 2u);
        if (selector == 2u) {
            fixed = true;
        } else {
            const uint8_t first = GP4_GLOBAL(uint8_t, 0x0066d896);
            flags = cmpFlags8(carByte, first);
            if (carByte == first) {
                fixed = true;
            } else if (GP4_GLOBAL(uint8_t, 0x007aafd4) != 0u) {
                const uint8_t second = GP4_GLOBAL(uint8_t, 0x0066d87d);
                flags = cmpFlags8(carByte, second);
                fixed = carByte == second;
            }
        }
        if (fixed) {
            result = 0x4000u;
        } else {
            gp4::call_regs(0x0043cdfe, call);
            result = call.eax;
            flags = call.eflags & 0x8d5u;
        }
    } else {
        gp4::call_regs(0x0043cdfe, call);
        const int64_t product = static_cast<int64_t>(static_cast<int32_t>(call.eax)) * static_cast<int64_t>(static_cast<int32_t>(value));
        edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
        result = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
        flags = shrd14Flags(static_cast<uint64_t>(product));
    }

    r->eax = result;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043ced2, FUN_0043ced2, "regs(edx:u32, esi:ptr[0x84]:layout=car_0043ced2) -> (eax, edx, cf, pf, af, zf, sf, of); cases=600; globals=fuzz:i32+layout=state_0066d8c0+words_0062d210+state_0042595c+tabs_0043ced2")
