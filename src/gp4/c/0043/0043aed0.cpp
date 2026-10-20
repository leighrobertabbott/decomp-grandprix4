#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of "cmp left, right" (32-bit).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// 0x78-byte object records at 0x0063bd7c (only the fields this routine reads).
struct Unknown_0063bd7c {
    uint32_t Unknown00;
    uint32_t Unknown04;
    uint32_t Unknown08;
    uint8_t Unknown0C[0x64 - 0x0C];
    uint32_t Unknown64;   // byte offset of the car record inside the 0x008943a0 ring
    uint8_t Unknown68[0x78 - 0x68];
};
static_assert(sizeof(Unknown_0063bd7c) == 0x78, "record stride");

// 0x160-byte records at 0x008943a0 (only the words this routine reads).
struct Unknown_008943a0 {
    uint8_t Unknown00[0x16];
    int16_t Unknown16;
    uint8_t Unknown18[0x60 - 0x18];
    uint16_t Unknown60;
    uint16_t Unknown62;
    uint8_t Unknown64[0x160 - 0x64];
};
static_assert(sizeof(Unknown_008943a0) == 0x160, "record stride");

// FUNCTION: GP4 0x0043aed0
// pushal/popal wrapper (every GPR is preserved; the flags are those of the last flag writer).
// Walks the object records 0x16..[0x006097ec] whose flag byte [0x0063bcfc + i] is set and whose
// car record (0x008943a0 + rec.Unknown64) lies below [0x00891500]. For each, 0x0040460a rotates
// the record's position into the car frame (result in 0x00934df0); the car is appended to the
// pointer list at 0x00607ba0 when the lateral offset is under [0x00607b98] or the forward
// distance is inside the car's lane words. [0x00607b9c] receives the list length.
void __cdecl FUN_0043aed0(gp4::Regs* r) {
    GP4_GLOBAL(int32_t, 0x00607b9c) = 0;
    const int32_t last = GP4_GLOBAL(int32_t, 0x006097ec);
    uint32_t flags;
    if (last < 0x16) {
        flags = subFlags32((uint32_t)last, 0x16u);   // cmp eax, 0x16 ; jl
    } else {
        int32_t count = 0;
        int32_t index = 0x16;
        do {
            if (GP4_ARRAY(uint8_t, 0x0063bcfc)[index] != 0u) {
                const Unknown_0063bd7c* rec = GP4_ARRAY(Unknown_0063bd7c, 0x0063bd7c) + index;
                const uint32_t carAddress = rec->Unknown64 + 0x008943a0u;
                if (carAddress < GP4_GLOBAL(uint32_t, 0x00891500)) {
                    const Unknown_008943a0* car = (const Unknown_008943a0*)(uintptr_t)carAddress;
                    GP4_GLOBAL(uint32_t, 0x00623e30) = rec->Unknown00;
                    GP4_GLOBAL(uint32_t, 0x00623e38) = rec->Unknown08;

                    gp4::Regs callee = *r;
                    callee.edi = carAddress;
                    callee.st_in = 0;
                    callee.st_out = 0;
                    gp4::call_regs(0x0040460a, callee);

                    int32_t lateral = (GP4_GLOBAL(int32_t, 0x00934df0) >> 8) - (int32_t)car->Unknown16;
                    if (lateral < 0) {
                        lateral = (int32_t)(0u - (uint32_t)lateral);
                    }
                    bool listed = lateral < GP4_GLOBAL(int32_t, 0x00607b98);
                    if (!listed) {
                        int32_t forward = GP4_GLOBAL(int32_t, 0x00934df0);
                        uint32_t edge;
                        if (forward < 0) {
                            forward = (int32_t)(0u - (uint32_t)forward);
                            edge = car->Unknown60;
                        } else {
                            edge = car->Unknown62;
                        }
                        listed = ((forward >> 8) - 0x100) < (int32_t)edge;
                    }
                    if (listed) {
                        GP4_ARRAY(uint32_t, 0x00607ba0)[count] = carAddress;
                        ++count;
                    }
                }
            }
            ++index;
        } while (index <= GP4_GLOBAL(int32_t, 0x006097ec));
        GP4_GLOBAL(int32_t, 0x00607b9c) = count;
        flags = subFlags32((uint32_t)index, GP4_GLOBAL(uint32_t, 0x006097ec));   // cmp ecx, [0x6097ec] ; jle
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043aed0, FUN_0043aed0, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=ring_008943a0+words_008943a0+bytes_0063bcfc+list_0043aed0")
