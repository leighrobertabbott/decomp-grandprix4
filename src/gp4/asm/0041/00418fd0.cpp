#include <gp4/gp4.h>

struct Unknown_00418fd0 {
    uint8_t Unknown00[0x2ac];
    uint32_t Deadline2AC[3];   // tick count + delay, one per slot
    uint8_t Tag2B8[3];         // slot owner tag (0 = free)
};

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00418fd0
// Claims the first free (tag 0) of three slots at esi+0x2b8 for tag cl+1 and schedules it at
// [0x66d814] + eax + ((random byte << 6) * edx >> 14). If a slot already holds the tag, or all three
// are taken, nothing changes and CF is set. All registers are preserved (pushal/popal); the
// flags are those of the last compare (equal: ZF|PF) or of the final add, with CF set/cleared.
void __cdecl FUN_00418fd0(gp4::Regs* r) {
    Unknown_00418fd0* rec = reinterpret_cast<Unknown_00418fd0*>(static_cast<uintptr_t>(r->esi));
    const uint8_t tag = static_cast<uint8_t>(r->ecx + 1u);
    const uint32_t delay = r->eax;
    const int32_t scale = static_cast<int32_t>(r->edx);

    uint32_t flags = 0x45u;   // cmp equal (ZF|PF), then stc

    for (uint32_t slot = 0; slot != 3; ++slot) {
        if (rec->Tag2B8[slot] == 0) {
            rec->Tag2B8[slot] = tag;

            gp4::Regs rng = *r;
            gp4::call_regs(0x00401462, rng);

            const int32_t draw = static_cast<int32_t>((rng.eax & 0xffu) << 6);
            const int64_t product = static_cast<int64_t>(draw) * scale;
            const uint32_t offset = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14) + delay;
            const uint32_t now = GP4_GLOBAL(uint32_t, 0x0066d814);

            rec->Deadline2AC[slot] = now + offset;
            flags = additionFlags32(now, offset) & ~1u;   // clc
            break;
        }
        if (rec->Tag2B8[slot] == tag) {
            break;
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00418fd0, FUN_00418fd0, "regs(eax:u32, ecx:u32[0..3], edx:i32, esi:ptr[0x2c4]:bytes) -> (cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0")
