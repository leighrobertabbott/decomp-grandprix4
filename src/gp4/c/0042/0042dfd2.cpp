#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of "cmp left, right" (32-bit).
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Arithmetic flags of "cmp left, right" (16-bit).
static inline uint32_t cmpFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Flags of "inc value" (CF is not touched); `result` = value + 1.
static inline uint32_t incFlags32(uint32_t value, uint32_t result) {
    return parityFlag(result)
        | (((value ^ 1u ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (result == 0x80000000u ? 0x800u : 0u);
}

// Car records (0x41c bytes) reached through the pointer lists (only the fields this routine reads).
struct Unknown_Car {
    uint8_t Unknown00[0x68];
    int16_t Unknown68;
    uint8_t Unknown6A[0x9c - 0x6a];
    uint8_t Flags9C;
    uint8_t Unknown9D[0xc6 - 0x9d];
    uint8_t UnknownC6;
    uint8_t UnknownC7[0xe2 - 0xc7];
    int16_t UnknownE2;
};

// FUNCTION: GP4 0x0042dfd2
// esi = the reference car, edi = the number of entries already in the pointer list 0x00604294,
// bp = a 16-bit lap-distance limit. 0x0042df3a classifies the esi car (cl = class). The car-pointer
// table 0x00607968 is walked downwards from index [0x00607964] - 1 to [0x00607960] for the first car
// with +0x68 <= bp and byte +0xc6 equal to the class. That car is appended (edi += 1) unless it is
// null, [0x00604280], already in the list, or its +0xe2 word (or the 0x0040dc8a distance when either
// car has bit 1 of +0x9c) minus the esi car's is below the word 0x00603b28. On exit eax, ebx, edx, edi
// and the flags are those of the exit path taken; ecx and ebp are unchanged.
void __cdecl FUN_0042dfd2(gp4::Regs* r) {
    const Unknown_Car* reference = (const Unknown_Car*)(uintptr_t)r->esi;
    const int16_t limit = (int16_t)r->ebp;
    uint32_t eax = r->eax;
    uint32_t ebx = r->ebx;
    int32_t edx = GP4_GLOBAL(int32_t, 0x00607964) - 1;
    uint32_t edi = r->edi;
    uint32_t flags;

    if (edx < GP4_GLOBAL(int32_t, 0x00607960)) {
        flags = cmpFlags32((uint32_t)edx, GP4_GLOBAL(uint32_t, 0x00607960));
        goto done;
    }

    {
        gp4::Regs classify = *r;
        classify.edx = (uint32_t)edx;
        classify.st_in = 0;
        classify.st_out = 0;
        gp4::call_regs(0x0042df3a, classify);
        eax = classify.eax;
    }
    const uint8_t wanted = (uint8_t)eax;

    for (;;) {
        ebx = GP4_ARRAY(uint32_t, 0x00607968)[edx];
        const Unknown_Car* candidate = (const Unknown_Car*)(uintptr_t)ebx;
        if (!(limit < candidate->Unknown68) && wanted == candidate->UnknownC6) {
            break;
        }
        --edx;
        if (edx < GP4_GLOBAL(int32_t, 0x00607960)) {
            flags = cmpFlags32((uint32_t)edx, GP4_GLOBAL(uint32_t, 0x00607960));
            goto done;
        }
    }

    {
        const Unknown_Car* candidate = (const Unknown_Car*)(uintptr_t)ebx;
        if (ebx == 0u) {
            flags = 0x44u;   // or ebx, ebx on zero
            goto done;
        }
        if (ebx == GP4_GLOBAL(uint32_t, 0x00604280)) {
            flags = 0x44u;   // cmp of equal values
            goto done;
        }
        if (limit < candidate->Unknown68) {
            flags = cmpFlags16((uint16_t)limit, (uint16_t)candidate->Unknown68);
            goto done;
        }

        int32_t index = (int32_t)edi - 1;
        eax = (uint32_t)index;
        if (index >= 0) {
            do {
                if (ebx == GP4_ARRAY(uint32_t, 0x00604294)[index]) {
                    eax = (uint32_t)index;
                    flags = 0x44u;
                    goto done;
                }
                --index;
            } while (index >= 0);
            eax = (uint32_t)index;
        }

        uint32_t word;
        if ((reference->Flags9C & 2u) != 0u || (candidate->Flags9C & 2u) != 0u) {
            gp4::Regs distance = *r;
            distance.eax = eax;
            distance.ebx = ebx;
            distance.ecx = (r->ecx & 0xffffff00u) | wanted;
            distance.edx = (uint32_t)edx;
            distance.st_in = 0;
            distance.st_out = 0;
            gp4::call_regs(0x0040dc8a, distance);
            eax = distance.eax;
            word = distance.eax & 0xffffu;
        } else {
            word = (uint16_t)((uint16_t)candidate->UnknownE2 - (uint16_t)reference->UnknownE2);
            eax = (eax & 0xffff0000u) | word;
        }

        const uint32_t threshold = GP4_GLOBAL(uint16_t, 0x00603b28);
        flags = cmpFlags16(word, threshold);
        if ((int16_t)word >= (int16_t)threshold) {
            GP4_ARRAY(uint32_t, 0x00604294)[edi] = ebx;
            const uint32_t next = edi + 1u;
            flags = (flags & 1u) | incFlags32(edi, next);
            edi = next;
        }
    }

done:
    r->eax = eax;
    r->ebx = ebx;
    r->edx = (uint32_t)edx;
    r->edi = edi;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042dfd2, FUN_0042dfd2, "regs(eax:u32, ebx:u32, esi:rec[0x66fef0,0x41c,0..21], edi:i32[0..8], ebp:i32[-4096..4096]) -> (eax, ebx, edx, edi, cf, pf, af, zf, sf, of); globals=layout=cars_0066fef0+pair_0042e8e6+list_0042dfd2")
