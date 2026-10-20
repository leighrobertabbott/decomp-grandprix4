#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the byte compare `cmp left, right`.
static inline uint32_t compareFlags8(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u);
}

// Flags of the dword compare `cmp left, right`.
static inline uint32_t compareFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// In race mode 'D' with at least two cars, counts the entries of the table at 0x00675970 that
// differ from 0x20000000; when at least a quarter of the cars have one, the byte at +0x7c selects
// tag 6, 7 or 8 (compared with the bytes at 0x00675958..0x0067595b, tags 7 and 8 only with at
// least 3 and 5 cars) and the delayed slot 0x00418fd0 is started with it unless byte 0x007c2e75 is
// set. Registers: eax is the tag (or the count with its low byte replaced by [esi+0x7c] on late
// exits), ecx = 0 and edx = cars >> 2 once the count is made; the flags are those of the last compare.
// FUNCTION: GP4 0x0040ee7b
void __cdecl FUN_0040ee7b(gp4::Regs* r) {
    const uint8_t* self = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));

    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    if (mode != 0x44u) {
        r->eflags = (r->eflags & ~0x8d5u) | compareFlags8(mode, 0x44u);
        return;
    }

    const int32_t cars = GP4_GLOBAL(int32_t, 0x0066fd6c);
    if (cars < 2) {
        r->eax = static_cast<uint32_t>(cars);
        r->eflags = (r->eflags & ~0x8d5u) | compareFlags32(static_cast<uint32_t>(cars), 2u);
        return;
    }

    const uint32_t* table = GP4_ARRAY(uint32_t, 0x00675970);
    uint32_t used = 0u;
    for (uint32_t i = 0; i != static_cast<uint32_t>(cars); ++i) {
        if (table[i] != 0x20000000u) {
            ++used;
        }
    }
    const uint32_t quarter = static_cast<uint32_t>(cars) >> 2;
    r->ecx = 0u;
    r->edx = quarter;
    if (static_cast<int32_t>(used) < static_cast<int32_t>(quarter)) {
        r->eax = used;
        r->eflags = (r->eflags & ~0x8d5u) | compareFlags32(used, quarter);
        return;
    }

    const uint8_t id = self[0x7c];
    const uint32_t lateEax = (used & 0xffffff00u) | id;
    uint32_t tag;
    uint32_t flags = compareFlags8(id, GP4_GLOBAL(uint8_t, 0x00675958));
    if (id == GP4_GLOBAL(uint8_t, 0x00675958)) {
        tag = 6u;
    } else {
        flags = compareFlags32(static_cast<uint32_t>(cars), 3u);
        if (cars < 3) {
            r->eax = lateEax;
            r->eflags = (r->eflags & ~0x8d5u) | flags;
            return;
        }
        flags = compareFlags8(id, GP4_GLOBAL(uint8_t, 0x00675959));
        if (id == GP4_GLOBAL(uint8_t, 0x00675959)) {
            tag = 7u;
        } else {
            flags = compareFlags32(static_cast<uint32_t>(cars), 5u);
            if (cars < 5) {
                r->eax = lateEax;
                r->eflags = (r->eflags & ~0x8d5u) | flags;
                return;
            }
            flags = compareFlags8(id, GP4_GLOBAL(uint8_t, 0x0067595a));
            if (id != GP4_GLOBAL(uint8_t, 0x0067595a)) {
                flags = compareFlags8(id, GP4_GLOBAL(uint8_t, 0x0067595b));
                if (id != GP4_GLOBAL(uint8_t, 0x0067595b)) {
                    r->eax = lateEax;
                    r->eflags = (r->eflags & ~0x8d5u) | flags;
                    return;
                }
            }
            tag = 8u;
        }
    }

    r->eax = tag;
    const uint8_t muted = GP4_GLOBAL(uint8_t, 0x007c2e75);
    flags = compareFlags8(muted, 0u);
    if (muted == 0u) {
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        call.ecx = tag;
        call.eax = 0x7d0u;
        call.edx = 0x1388u;
        gp4::call_regs(0x00418fd0, call);
        flags = call.eflags & 0x8d5u;
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040ee7b, FUN_0040ee7b, "regs(eax:u32, ecx:u32, edx:u32, esi:ptr[0x2c4]:layout=rec_0040ee7b) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=state_0040ee7b")
