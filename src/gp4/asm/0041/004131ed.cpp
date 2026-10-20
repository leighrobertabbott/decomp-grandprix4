#include <gp4/gp4.h>

// FUNCTION: GP4 0x004131ed
void __cdecl FUN_004131ed(gp4::Regs* r) {
    // Blends the four dwords at esi+0x2bc towards the four at esi+0x2ec (14-bit fixed point):
    // new = (old * (0x4000 - w) + target * w) >> 14, with w = min(0x4000, (rate * [0x60384c]) >> 10)
    // and rate chosen by whether the target is below the old value.
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const int32_t rateFalling = GP4_GLOBAL(int32_t, 0x006030f0);
    const int32_t rateRising = GP4_GLOBAL(int32_t, 0x006030ec);

    uint32_t eax = 0;
    uint32_t edx = 0;
    uint32_t ebp = 0;
    for (uint32_t i = 0; i != 4; ++i) {
        const int32_t target = GP4_FIELD(int32_t, body, 0x2ec + i * 4);
        const int32_t old = GP4_FIELD(int32_t, body, 0x2bc + i * 4);
        const int32_t rate = target < old ? rateFalling : rateRising;
        const uint64_t scaled = static_cast<uint64_t>(static_cast<int64_t>(rate) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0060384c)));
        int32_t weight = static_cast<int32_t>(static_cast<uint32_t>(scaled >> 10));
        if (weight > 0x4000) weight = 0x4000;
        const uint64_t towards = static_cast<uint64_t>(static_cast<int64_t>(target) * static_cast<int64_t>(weight));
        ebp = static_cast<uint32_t>(towards >> 14);
        const uint64_t remain = static_cast<uint64_t>(static_cast<int64_t>(0x4000 - weight) * static_cast<int64_t>(old));
        edx = static_cast<uint32_t>(remain >> 32);
        eax = static_cast<uint32_t>(remain >> 14) + ebp;
        GP4_FIELD(uint32_t, body, 0x2bc + i * 4) = eax;
    }
    r->eax = eax;
    r->ecx = 4;
    r->edx = edx;
    r->ebp = ebp;
    // cmp ecx, 4 with ecx == 4: ZF = PF = 1, the rest clear (ebx is pushed and popped)
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x004131ed, FUN_004131ed, "regs(esi:ptr[0x300]:bytes) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
