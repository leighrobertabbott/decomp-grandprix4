#include <gp4/gp4.h>

struct Unknown_0043d909 {
    uint8_t Unknown00[0x98];
    uint32_t Unknown98;          // its high word (+0x9a) is read back as a word after the store
    uint8_t Unknown9c[0xee - 0x9c];
    uint16_t UnknownEe;
};

// FUNCTION: GP4 0x0043d909
// Clamps the word at esi+0x9a to 0x5800, runs 0x00426a66 and 0x00425eca (the latter with the word at
// +0xee in 0x00603d40) and updates the dword at +0x98 with (result * 4 - [0x006037fc]), floored at 0.
// The 16-bit difference between the new high word (+0x9a) and the clamped value, scaled by
// 0x800 << 6, is divided (idiv by the low word of [0x00603854], guarded against overflow by its
// half) and the quotient is combined with the square of the old word; 0x00409c44 turns that dx:ax
// pair into the new word at +0x9a. ebx, ecx and ebp are restored; eax, edx and the flags are the
// 0x00409c44 results.
void __cdecl FUN_0043d909(gp4::Regs* r) {
    Unknown_0043d909* rec = reinterpret_cast<Unknown_0043d909*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs rg = *r;
    rg.st_in = 0;
    rg.st_out = 0;
    rg.st_xin_mask = 0;
    rg.st_xout_mask = 0;

    const uint16_t oldWord = GP4_FIELD(uint16_t, rec, 0x9a);
    uint16_t clamped = oldWord;
    if (static_cast<int16_t>(clamped) > 0x5800) {
        clamped = 0x5800;
    }
    GP4_FIELD(uint16_t, rec, 0x9a) = clamped;

    gp4::call_regs(0x00426a66, rg);

    GP4_GLOBAL(uint16_t, 0x00934df4) = 0x6000;
    rg.eax = rec->UnknownEe;
    GP4_GLOBAL(uint32_t, 0x00603d40) = rg.eax;
    GP4_GLOBAL(uint32_t, 0x00603d48) = 0;
    gp4::call_regs(0x00425eca, rg);

    const uint32_t delta = (rg.eax << 2) - GP4_GLOBAL(uint32_t, 0x006037fc);
    int32_t newValue = static_cast<int32_t>(rec->Unknown98 + delta);
    if (newValue < 0) {
        newValue = 0;
    }
    rec->Unknown98 = static_cast<uint32_t>(newValue);
    const uint32_t edxHigh = static_cast<uint32_t>(newValue) & 0xffff0000u;
    const uint32_t eaxHigh = delta & 0xffff0000u;

    // ax = word[+0x9a] - bx (16 bit), then dx:ax = -ax * 0x800 shifted left by 6.
    const uint16_t diff = static_cast<uint16_t>(GP4_FIELD(uint16_t, rec, 0x9a) - clamped);
    const int32_t square = static_cast<int32_t>(static_cast<int16_t>(oldWord)) * static_cast<int32_t>(static_cast<int16_t>(oldWord));
    const uint16_t squareLow = static_cast<uint16_t>(square);
    const uint16_t squareHigh = static_cast<uint16_t>(static_cast<uint32_t>(square) >> 16);

    const int32_t scaled = static_cast<int32_t>(static_cast<int16_t>(0u - diff)) * 0x800;
    const uint32_t shifted = static_cast<uint32_t>(scaled) << 6;
    const uint16_t lowWord = static_cast<uint16_t>(shifted);
    const int16_t highWord = static_cast<int16_t>(shifted >> 16);

    // bp = [0x00603854] halved toward zero (sar 1, +1 when the result is negative).
    const uint32_t divisorFull = GP4_GLOBAL(uint32_t, 0x00603854);
    int16_t half = static_cast<int16_t>(static_cast<int16_t>(divisorFull) >> 1);
    if (half < 0) {
        half = static_cast<int16_t>(half + 1);
    }

    bool divide;
    if (highWord < 0) {
        if (half >= 0) {
            const int16_t negHalf = static_cast<int16_t>(-half);
            divide = (highWord > negHalf) || (highWord == negHalf && lowWord != 0);
        } else {
            divide = (highWord > half) || (highWord == half && lowWord != 0);
        }
    } else if (half < 0) {
        const int16_t negHalf = static_cast<int16_t>(0 - half);
        divide = highWord < negHalf;
    } else {
        divide = highWord < half;
    }

    uint16_t quotient = 0;
    if (divide) {
        const int32_t dividend = static_cast<int32_t>((static_cast<uint32_t>(static_cast<uint16_t>(highWord)) << 16) | lowWord);
        quotient = static_cast<uint16_t>(dividend / static_cast<int16_t>(divisorFull));
    }

    // cwd; dh = dl; dl = ah; ah = al; al = 0; then two more shl ax / rcl dx: (sign-extended quotient) << 10.
    const uint32_t widened = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(quotient))) << 10;
    const uint32_t lowSum = static_cast<uint32_t>(static_cast<uint16_t>(widened)) + squareLow;
    const uint16_t axOut = static_cast<uint16_t>(lowSum);
    const uint16_t dxOut = static_cast<uint16_t>(static_cast<uint16_t>(widened >> 16) + squareHigh + (lowSum >> 16));

    rg.eax = eaxHigh | axOut;
    rg.edx = edxHigh | dxOut;
    gp4::call_regs(0x00409c44, rg);
    GP4_FIELD(uint16_t, rec, 0x9a) = static_cast<uint16_t>(rg.eax);

    r->eax = rg.eax;
    r->edx = rg.edx;
    r->eflags = rg.eflags;
}
GP4_IMPL(0x0043d909, FUN_0043d909, "regs(esi:ptr[0x267]:layout=car_0043d7be) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0043d7be+torque_00425e19+frac_00624790")
