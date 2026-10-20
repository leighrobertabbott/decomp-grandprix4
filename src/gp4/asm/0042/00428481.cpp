#include <gp4/gp4.h>

// esi: a car record (only the fields read here are named).
struct Unknown_00428481_Car {
    uint8_t Unknown00[0x1c];
    uint32_t Segment1C;         // current track-segment record (array at 0x8943a0, stride 0x160)
    uint8_t Unknown20[0x98 - 0x20];
    int32_t Unknown98;
};

// A track-segment record.
struct Unknown_00428481_Segment {
    uint8_t Unknown00[0x51];
    uint8_t Flags51;
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `a - b` (cmp a, b / neg with a = 0).
static inline uint32_t subFlags(uint32_t a, uint32_t b) {
    const uint32_t res = a - b;
    return (a < b ? 0x001u : 0u) | parityFlag(res) | ((a ^ b ^ res) & 0x10u)
        | (res == 0u ? 0x40u : 0u) | ((res >> 31) << 7)
        | ((((a ^ b) & (a ^ res)) >> 31) << 11);
}

// Flags of a 32-bit test / logic operation: CF = OF = AF = 0.
static inline uint32_t logicFlags(uint32_t res) {
    return parityFlag(res) | (res == 0u ? 0x40u : 0u) | ((res >> 31) << 7);
}

// The two ratio routines are kept as separate call sites (the compiler would otherwise merge them
// into one call with a selected address).
static __declspec(noinline) void callStraight(gp4::Regs& call) {
    gp4::call_regs(0x00427f3c, call);
}

static __declspec(noinline) void callCurved(gp4::Regs& call) {
    gp4::call_regs(0x00428216, call);
}

// FUNCTION: GP4 0x00428481
// Lateral-offset magnitude of a car on its track segment: [0x62480c] = 0, then when the car's
// segment (+0x1c) lies below the pit start [0x891500] and has flag 0x20 in byte +0x51, the ratio from
// 0x00427f3c (or 0x00428216 when flag 8 is set) is scaled by the car's dword +0x98 (* [0x603854] >> 24,
// * ratio >> 12, * [0x6f2bf0] >> 14) and stored as an absolute value. eax = that value, edx the high
// half of the last product, ebp the callee's ratio, ecx whatever the callee leaves; the flags are
// those of the last compare / test / neg on the taken path. edi is preserved (push / pop).
void __cdecl FUN_00428481(gp4::Regs* r) {
    const Unknown_00428481_Car* car = reinterpret_cast<const Unknown_00428481_Car*>(static_cast<uintptr_t>(r->esi));

    GP4_GLOBAL(uint32_t, 0x0062480c) = 0;
    const uint32_t segmentAddr = car->Segment1C;
    const Unknown_00428481_Segment* segment =
        reinterpret_cast<const Unknown_00428481_Segment*>(static_cast<uintptr_t>(segmentAddr));

    uint32_t flags;
    if (segmentAddr >= GP4_GLOBAL(uint32_t, 0x00891500)) {
        flags = subFlags(segmentAddr, GP4_GLOBAL(uint32_t, 0x00891500));
    } else if ((segment->Flags51 & 0x20u) == 0u) {
        flags = logicFlags(0u);
    } else {
        gp4::Regs call = *r;
        call.edi = segmentAddr;
        call.st_in = 0;
        call.st_out = 0;
        if ((segment->Flags51 & 8u) == 0u) {
            callStraight(call);
        } else {
            callCurved(call);
        }
        const int32_t ratio = static_cast<int32_t>(call.eax);

        int64_t product = static_cast<int64_t>(car->Unknown98) * GP4_GLOBAL(int32_t, 0x00603854);
        int32_t value = static_cast<int32_t>(static_cast<uint64_t>(product) >> 24);
        product = static_cast<int64_t>(value) * ratio;
        value = static_cast<int32_t>(static_cast<uint64_t>(product) >> 12);
        product = static_cast<int64_t>(value) * GP4_GLOBAL(int32_t, 0x006f2bf0);
        value = static_cast<int32_t>(static_cast<uint64_t>(product) >> 14);

        flags = logicFlags(static_cast<uint32_t>(value));
        if (value < 0) {
            flags = subFlags(0u, static_cast<uint32_t>(value));
            value = static_cast<int32_t>(0u - static_cast<uint32_t>(value));
        }
        GP4_GLOBAL(uint32_t, 0x0062480c) = static_cast<uint32_t>(value);

        r->eax = static_cast<uint32_t>(value);
        r->ecx = call.ecx;
        r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
        r->ebp = static_cast<uint32_t>(ratio);
        r->ebx = call.ebx;
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00428481, FUN_00428481, "regs(esi:ptr[0x238]:layout=car_0043c677) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+walk_00427f3c+look_00428216")
