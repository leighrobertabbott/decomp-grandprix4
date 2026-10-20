#include <gp4/gp4.h>

struct Unknown_00405254_Engine {
    uint8_t Unknown00[0x98];
    int32_t Value98;
    uint8_t Unknown9C[0xac - 0x9c];
    int32_t ValueAC;
    uint8_t UnknownB0[0x2cc - 0xb0];
    uint32_t Divisor2CC[4];
    uint8_t Unknown2DC[0x32c - 0x2dc];
    int32_t Result32C[4];
};

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// FUNCTION: GP4 0x00405254
// Engine record in esi: [0x00603088] = (|+0xac| * [0x006f2bec] << 6) / max(|+0x98| >> 8, 0x80000),
// then the drivetrain 0x00404f99 (eax = that divisor, edx = the quotient) fills the four dwords at
// 0x0060309c, each of which is divided (<< 6) by the divisor dword +0x2cc+4i and stored << 4 at
// +0x32c+4i. Finally [0x00623f10] = ((((+0x98 >> 8)^2) >> 14) * [0x00603088]) >> 28. eax is that
// result, ebp the divisor, ecx 4, edx the high dword of the last product, flags those of the last shrd.
void __cdecl FUN_00405254(gp4::Regs* r) {
    Unknown_00405254_Engine* engine = reinterpret_cast<Unknown_00405254_Engine*>(static_cast<uintptr_t>(r->esi));

    const uint32_t speed = static_cast<uint32_t>(engine->Value98);
    uint32_t magnitude = static_cast<int32_t>(speed) < 0 ? 0u - speed : speed;
    int32_t divisor = static_cast<int32_t>(magnitude) >> 8;
    if (divisor < 0x80000) {
        divisor = 0x80000;
    }

    const uint32_t load = static_cast<uint32_t>(engine->ValueAC);
    const uint32_t loadMagnitude = static_cast<int32_t>(load) < 0 ? 0u - load : load;
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(loadMagnitude)) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006f2bec));
    const int64_t dividend = static_cast<int64_t>(static_cast<uint64_t>(product) << 6);
    int32_t remainder;
    GP4_GLOBAL(int32_t, 0x00603088) = gp4::idiv64(dividend, divisor, &remainder);

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.ebp = static_cast<uint32_t>(divisor);
    call.eax = static_cast<uint32_t>(divisor);
    call.edx = GP4_GLOBAL(uint32_t, 0x00603088);
    gp4::call_regs(0x00404f99, call);

    for (uint32_t i = 0; i != 4; ++i) {
        const uint32_t term = GP4_ARRAY(uint32_t, 0x0060309c)[i];
        uint32_t rem;
        const uint32_t quotient = gp4::div64(static_cast<uint64_t>(term) << 6, engine->Divisor2CC[i], &rem);
        engine->Result32C[i] = static_cast<int32_t>(quotient << 4);
    }

    const int32_t shifted = engine->Value98 >> 8;
    const int64_t square = static_cast<int64_t>(shifted) * static_cast<int64_t>(shifted);
    const uint32_t squareScaled = static_cast<uint32_t>(static_cast<uint64_t>(square) >> 14);
    const int64_t last = static_cast<int64_t>(static_cast<int32_t>(squareScaled)) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603088));
    const uint32_t lo = static_cast<uint32_t>(static_cast<uint64_t>(last));
    const uint32_t hi = static_cast<uint32_t>(static_cast<uint64_t>(last) >> 32);
    const uint32_t result = static_cast<uint32_t>(static_cast<uint64_t>(last) >> 28);
    const uint32_t previous = static_cast<uint32_t>(static_cast<uint64_t>(last) >> 27);
    GP4_GLOBAL(uint32_t, 0x00623f10) = result;

    const uint32_t flags = ((lo >> 27) & 1u)
        | parityFlag8(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((previous ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);

    gp4::Regs out = call;
    out.eax = result;
    out.ecx = 4u;
    out.edx = hi;
    out.ebp = static_cast<uint32_t>(divisor);
    out.eflags = (r->eflags & ~0x8d5u) | flags;
    *r = out;
}
GP4_IMPL(0x00405254, FUN_00405254, "regs(esi:ptr[0x358]:layout=rec_00405254) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_00404ed0+state_00404f33+engine_00413606")
