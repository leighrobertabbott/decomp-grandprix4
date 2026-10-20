#include <gp4/gp4.h>

struct Unknown_00415d1c_Car {
    uint8_t Unknown00[0x62];
    uint16_t Unknown62;
    uint8_t Unknown64[0x72 - 0x64];
    uint8_t Unknown72;
    uint8_t Unknown73[0x78 - 0x73];
    uint8_t Flags78;
    uint8_t Unknown79[0x7b - 0x79];
    uint8_t Flags7B;
    uint8_t Unknown7C[0x9a - 0x7c];
    int16_t Unknown9A;
    uint8_t Unknown9C[0xee - 0x9c];
    uint16_t UnknownEE;
    uint8_t UnknownF0[0xfe - 0xf0];
    uint8_t FlagsFE;
    uint8_t UnknownFF[0x1c6 - 0xff];
    uint8_t Table1C6[0x231 - 0x1c6];
    uint8_t Flags231;
    uint8_t Flags232;
    uint8_t Unknown233[0x260 - 0x233];
    int32_t Unknown260;
};

// edx:eax = eax * factor (imul), then eax = the low dword of that product >> shift (shrd eax, edx).
static inline void imulShrd(gp4::Regs* r, int32_t factor, int shift) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(r->eax)) * static_cast<int64_t>(factor);
    r->eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> shift);
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
}

// 0x00415eec: shape the value through 0x00415baf and store it into word +0x62.
static inline void finish(gp4::Regs* r, Unknown_00415d1c_Car* car) {
    gp4::call_regs(0x00415baf, *r);
    car->Unknown62 = static_cast<uint16_t>(r->eax);
}

// 0x00415ec8: a random byte * 64 scaled by [0x00603a60] (Q14) on top of [0x00603a5c].
static inline void randomValue(gp4::Regs* r, Unknown_00415d1c_Car* car) {
    gp4::call_regs(0x00401462, *r);
    r->eax = (r->eax & 0xffu) << 6;
    imulShrd(r, GP4_GLOBAL(int32_t, 0x00603a60), 14);
    r->eax += GP4_GLOBAL(uint32_t, 0x00603a5c);
    finish(r, car);
}

// 0x00415e74: word +0xee cleared; above [0x00603a5c] + [0x00603a60] the value steps down by
// [0x00603a50] (or [0x00603a54] below [0x00603a58]) scaled by [0x00603854] >> 16, unless that
// drops below [0x00603a5c], in which case (as at or below the band) a random value is drawn.
static inline void lowPath(gp4::Regs* r, Unknown_00415d1c_Car* car) {
    car->UnknownEE = 0;
    r->edx = GP4_GLOBAL(uint32_t, 0x00603a5c) + GP4_GLOBAL(uint32_t, 0x00603a60);
    r->eax = car->Unknown62;
    if (static_cast<int32_t>(r->eax) <= static_cast<int32_t>(r->edx)) {
        randomValue(r, car);
        return;
    }
    GP4_GLOBAL(uint8_t, 0x00603a78) = 0xff;
    r->eax = GP4_GLOBAL(uint32_t, 0x00603a50);
    r->edx = car->Unknown62;
    if (static_cast<int32_t>(r->edx) < GP4_GLOBAL(int32_t, 0x00603a58)) {
        r->eax = GP4_GLOBAL(uint32_t, 0x00603a54);
    }
    imulShrd(r, GP4_GLOBAL(int32_t, 0x00603854), 16);
    r->eax = 0u - r->eax;
    r->edx = car->Unknown62;
    r->eax += r->edx;
    if (static_cast<int32_t>(r->eax) >= GP4_GLOBAL(int32_t, 0x00603a5c)) {
        finish(r, car);
        return;
    }
    randomValue(r, car);
}

// 0x00415e3c: the rate limiter 0x00415b00 gives the value; its edx goes to word +0xee.
static inline void rateLimited(gp4::Regs* r, Unknown_00415d1c_Car* car) {
    gp4::call_regs(0x00415b00, *r);
    car->UnknownEE = static_cast<uint16_t>(r->edx);
    finish(r, car);
}

// 0x00415e32: new random target (0x00415cac, 0x00415bde), then the rate limiter.
static inline void retarget(gp4::Regs* r, Unknown_00415d1c_Car* car) {
    gp4::call_regs(0x00415cac, *r);
    gp4::call_regs(0x00415bde, *r);
    rateLimited(r, car);
}

// 0x00415e4d: hold - bit 1 of +0x231 set, word +0xee = 0x4000, value = +0x62 plus
// [0x00603a4c] scaled by [0x00603854] >> 16.
static inline void hold(gp4::Regs* r, Unknown_00415d1c_Car* car) {
    car->Flags231 = static_cast<uint8_t>(car->Flags231 | 2u);
    car->UnknownEE = 0x4000;
    r->eax = GP4_GLOBAL(uint32_t, 0x00603a4c);
    imulShrd(r, GP4_GLOBAL(int32_t, 0x00603854), 16);
    r->edx = car->Unknown62;
    r->eax += r->edx;
    finish(r, car);
}

// esi = car record. Computes the target [0x00603a44] = (0x0041242f of the clamped byte +0x72 over
// the table at +0x1c6) * word +0x9a >> 14 and updates word +0x62 towards it.
// FUNCTION: GP4 0x00415d1c
void __cdecl FUN_00415d1c(gp4::Regs* r) {
    Unknown_00415d1c_Car* car = reinterpret_cast<Unknown_00415d1c_Car*>(static_cast<uintptr_t>(r->esi));

    GP4_GLOBAL(uint8_t, 0x00603a78) = 0;
    r->ecx = static_cast<uint32_t>(static_cast<int32_t>(car->Unknown9A));
    int8_t count = static_cast<int8_t>(car->Unknown72);
    if (count < 1) {
        count = 1;
    }
    r->eax = (r->eax & 0xffffff00u) | static_cast<uint8_t>(count);
    car->Unknown72 = static_cast<uint8_t>(count);
    r->edx = r->esi + 0x1c6u;
    gp4::call_regs(0x0041242f, *r);
    imulShrd(r, static_cast<int32_t>(r->ecx), 14);
    GP4_GLOBAL(uint32_t, 0x00603a44) = r->eax;

    if ((car->Flags7B & 2u) == 0u) {
        r->eax = 0;
        finish(r, car);
        return;
    }
    if ((car->FlagsFE & 0x20u) != 0u) {
        gp4::call_regs(0x00415a43, *r);
        finish(r, car);
        return;
    }
    if (GP4_GLOBAL(uint8_t, 0x007c2ea9) != 0x80u) {
        r->eax = GP4_GLOBAL(uint32_t, 0x00603a44);
        if (static_cast<int32_t>(r->eax) >= GP4_GLOBAL(int32_t, 0x00603a5c)) {
            finish(r, car);
            return;
        }
        lowPath(r, car);
        return;
    }
    if (GP4_GLOBAL(uint8_t, 0x0066d88c) < 3u) {
        lowPath(r, car);
        return;
    }

    car->Flags78 = static_cast<uint8_t>(car->Flags78 | 0x40u);
    // (the original compares esi with [0x00679d30] here; both outcomes continue identically)
    if ((car->Flags232 & 0x80u) == 0u) {
        car->Flags232 = static_cast<uint8_t>(car->Flags232 | 0x20u);
        car->Flags232 = static_cast<uint8_t>(car->Flags232 | 0x40u);
        car->Flags232 = static_cast<uint8_t>(car->Flags232 | 0x80u);
        retarget(r, car);
        return;
    }
    if ((car->Flags231 & 2u) != 0u) {
        gp4::call_regs(0x00415cef, *r);
        if ((r->eflags & gp4::CF) == 0u) {
            hold(r, car);
            return;
        }
        car->Flags231 = static_cast<uint8_t>(car->Flags231 & 0xfdu);
        car->Flags232 = static_cast<uint8_t>(car->Flags232 | 0x10u);
        car->Flags232 = static_cast<uint8_t>(car->Flags232 | 0x80u);
        retarget(r, car);
        return;
    }
    const int32_t due = car->Unknown260;
    if (due == 0) {
        rateLimited(r, car);
        return;
    }
    const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
    if (static_cast<int32_t>(clock - static_cast<uint32_t>(due)) < 0) {
        rateLimited(r, car);
        return;
    }
    r->eax = GP4_GLOBAL(uint32_t, 0x0063b834) - clock;
    if (static_cast<int32_t>(r->eax) < GP4_GLOBAL(int32_t, 0x00603a80)) {
        retarget(r, car);
        return;
    }
    gp4::call_regs(0x00401462, *r);
    r->eax = (r->eax & 0xffu) << 6;
    if (static_cast<int32_t>(r->eax) < GP4_GLOBAL(int32_t, 0x00603aa8)) {
        retarget(r, car);
        return;
    }
    hold(r, car);
}
GP4_IMPL(0x00415d1c, FUN_00415d1c, "regs(ebp:u32, esi:ptr[0x26c]:bytes) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0041242f+state_0066d8c0+state_0041e500")
