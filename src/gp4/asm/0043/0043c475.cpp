#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0043c475
// esi = record whose byte +0x7c holds a 6-bit slot number (1-based). The slot's cached byte in the
// table 0x0066d94c (index (slot - 1), capped at 0x15; slot 0 reads the byte before the table) is the
// answer unless it is 0xff (not loaded yet): then 0x0043bb15 loads the slot's block into [0x601f24]
// and the answer is the block's signed byte +0x32 when that is below 2, else the byte 0x0062d27c.
// Returns eax = min(answer, 1) (answer unsigned); the flags are those of the final "cmp eax, 1".
// ebx and edx are saved and restored; 0x0043bb15 is a pushal/popal wrapper.
void __cdecl FUN_0043c475(gp4::Regs* r) {
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));

    int32_t index = static_cast<int32_t>(*(record + 0x7c) & 0x3fu) - 1;
    if (index > 0x15) {
        index = 0x15;
    }
    uint8_t answer = GP4_ARRAY(uint8_t, 0x0066d94c)[index];
    if (answer == 0xffu) {
        gp4::Regs load = *r;
        load.st_in = 0;
        load.st_out = 0;
        load.eax = static_cast<uint32_t>(index);
        gp4::call_regs(0x0043bb15, load);

        const uint8_t* block = GP4_GLOBAL(uint8_t*, 0x00601f24);
        answer = *(block + 0x32);
        if (static_cast<int8_t>(answer) >= 2) {
            answer = GP4_GLOBAL(uint8_t, 0x0062d27c);
        }
    }

    const uint32_t value = answer;
    r->eax = value > 1u ? 1u : value;
    r->eflags = (r->eflags & ~0x8d5u) | cmpFlags32(value, 1u);
}
GP4_IMPL(0x0043c475, FUN_0043c475, "regs(esi:ptr[0x84]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trk_007ad894+circ_007adb50+tbl_0066d94c")
