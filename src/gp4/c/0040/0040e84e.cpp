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

// Flags of a byte `test` / `and` result (CF = OF = AF = 0).
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// FUNCTION: GP4 0x0040e84e
void __cdecl FUN_0040e84e(gp4::Regs* r) {
    uint8_t* self = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    const uint32_t count = static_cast<uint32_t>(GP4_GLOBAL(uint8_t, 0x0066d8c8)) + 1u;
    const uint32_t limit = static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x007c2e96));

    bool clear = false;
    bool arm = false;
    uint32_t flags = 0u;

    if (static_cast<int32_t>(count) >= static_cast<int32_t>(limit)) {
        clear = true;
    } else if ((GP4_FIELD(uint8_t, self, 0xfe) & 4u) != 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(GP4_FIELD(uint8_t, self, 0xfe) & 4u);
        return;
    } else {
        const uint32_t remaining = limit - count + 1u;
        const int32_t position = static_cast<int32_t>(GP4_FIELD(uint16_t, self, 0x60));
        if (position < GP4_GLOBAL(int32_t, 0x00602a6c)
            && position < static_cast<int32_t>((remaining << 6) - 0x10u)) {
            arm = true;
        } else {
            const int8_t cursor = GP4_FIELD(int8_t, self, 0x136);
            if (cursor >= GP4_FIELD(int8_t, self, 0x198)) {
                clear = true;
            } else {
                const int8_t entry = GP4_FIELD(int8_t, self, 0x199 + static_cast<uint32_t>(static_cast<uint8_t>(cursor)));
                if (entry > GP4_FIELD(int8_t, self, 0x70)) {
                    clear = true;
                } else {
                    gp4::Regs search = *r;
                    search.st_in = 0;
                    search.st_out = 0;
                    gp4::call_regs(0x0043b974, search);
                    uint8_t* other = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(search.ebx));
                    arm = true;
                    if ((search.eflags & gp4::CF) != 0u
                        && (GP4_FIELD(uint8_t, other, 0x71) & 0x80u) != 0u
                        && (GP4_FIELD(uint8_t, other, 0x8d) & 2u) == 0u) {
                        const uint8_t bit = GP4_FIELD(uint8_t, other, 0x71) & 2u;
                        if (bit != 0u) {
                            r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(bit);
                            return;
                        }
                    }
                }
            }
        }
    }

    if (clear) {
        GP4_FIELD(uint8_t, self, 0xfe) &= 0xfbu;
        GP4_FIELD(uint8_t, self, 0x71) &= 0x7fu;
        GP4_FIELD(uint8_t, self, 0x230) &= 0x7fu;
        flags = logicFlags8(GP4_FIELD(uint8_t, self, 0x230));
    } else if (arm) {
        GP4_FIELD(uint8_t, self, 0x71) |= 0x80u;
        GP4_FIELD(uint8_t, self, 0x230) &= 0x7fu;
        GP4_FIELD(uint8_t, self, 0xfe) |= 4u;
        const uint8_t muted = GP4_GLOBAL(uint8_t, 0x007c2e75);
        flags = compareFlags8(muted, 0u);
        if (muted == 0u) {
            gp4::Regs call = *r;
            call.st_in = 0;
            call.st_out = 0;
            call.ecx = 0xbu;
            call.eax = 0x3a98u;
            call.edx = 0x7530u;
            gp4::call_regs(0x00418fd0, call);
            flags = call.eflags & 0x8d5u;
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040e84e, FUN_0040e84e, "regs(esi:ptr[0x2d0]:layout=rec_0040e84e) -> (cf, pf, af, zf, sf, of); globals=layout=state_0040e84e")
