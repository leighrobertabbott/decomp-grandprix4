#include <gp4/gp4.h>

// A car record (array at 0x66fef0, stride 0x41c); only the fields used here are named.
struct Unknown_0043b689_Car {
    uint8_t Unknown00[0x66];
    int16_t Unknown66;
    uint8_t Unknown68[0x70 - 0x68];
    uint8_t Laps70;
    uint8_t Unknown71[0x7c - 0x71];
    uint8_t Flags7C;
    uint8_t Unknown7D[0x88 - 0x7d];
    uint8_t Flags88;
    uint8_t Unknown89[0x8d - 0x89];
    uint8_t Flags8D;
    uint8_t Unknown8E[0xf8 - 0x8e];
    int32_t Key0F8;
    uint8_t UnknownFC[0x415 - 0xfc];
    uint8_t Order415;
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// Flags of an 8-bit `or` (CF = OF = AF = 0).
static inline uint32_t orFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

static inline uint32_t setLow16(uint32_t full, uint32_t low) {
    return (full & 0xffff0000u) | (low & 0xffffu);
}

static inline Unknown_0043b689_Car* carAt(uint32_t address) {
    return reinterpret_cast<Unknown_0043b689_Car*>(static_cast<uintptr_t>(address));
}

// The car record named by the sign-extended word at `list` (a byte offset into the car array).
static __declspec(noinline) Unknown_0043b689_Car* carOfWord(const int16_t* list) {
    return carAt(0x0066fef0u + static_cast<uint32_t>(static_cast<int32_t>(*list)));
}

// FUNCTION: GP4 0x0043b689
// pushal / popal. Scores the 22 cars: the 32-bit product of words [0x007c2e96] * [0x00868558] is
// kept in [0x00604160]; 0x51f1d4 reports (CF) a mode that takes 0x1000 off the 0x43b482 distance of
// the cars whose flag byte +0x7c is not negative. A car with bit 2 of +0x8d, or a distance not
// below [0x00604160], gets max(laps - 1, 0) * [0x00868558] + 100 - byte +0x415 instead; the score
// goes to dword +0xf8 and the car's byte offset to the list [0x008914f8]. The list at 0x0071f9a4
// is then bubble-sorted by the score (descending) and, when equal, by the signed word +0x66; the
// listed cars get their rank * 2 in byte +0x415 and their flag byte +0x7c is copied to
// 0x007c31dc.., and bit 1 of byte +0x88 of the car [0x00679d30] is set. The flags that escape are
// those of that last `or`.
void __cdecl FUN_0043b689(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    const uint32_t product = static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x007c2e96)) * GP4_GLOBAL(uint16_t, 0x00868558);
    c.eax = setLow16(c.eax, product);
    c.edx = setLow16(c.edx, product >> 16);
    GP4_GLOBAL(uint16_t, 0x00604160) = static_cast<uint16_t>(product);
    GP4_GLOBAL(uint16_t, 0x00604162) = static_cast<uint16_t>(product >> 16);

    c.ebx = 0;
    gp4::call_regs(0x0051f1d4, c);
    if ((c.eflags & 1u) != 0u) {
        c.ebx = 0xffffffffu;
    }
    const uint32_t penalty = c.ebx;

    uint16_t* out = GP4_GLOBAL(uint16_t*, 0x008914f8);
    c.esi = 0x0066fef0u;
    c.ecx = 0x16u;
    do {
        c.ebx = penalty;
        gp4::call_regs(0x0043b482, c);
        c.ebx = penalty;

        c.eax = (c.edx << 16) | (c.eax & 0xffffu);
        Unknown_0043b689_Car* car = carAt(c.esi);
        if (penalty != 0u && static_cast<int8_t>(car->Flags7C) >= 0) {
            c.eax -= 0x1000u;
        }
        if ((car->Flags8D & 4u) != 0u || static_cast<int32_t>(c.eax) >= GP4_GLOBAL(int32_t, 0x00604160)) {
            const uint32_t rest = 100u - static_cast<uint32_t>(car->Order415);
            int32_t laps = static_cast<int32_t>(car->Laps70) - 1;
            if (laps < 0) {
                laps = 0;
            }
            c.eax = static_cast<uint32_t>(laps) * GP4_GLOBAL(uint16_t, 0x00868558);
            c.edx = rest;
            c.eax += c.edx;
        }
        car->Key0F8 = static_cast<int32_t>(c.eax);
        c.eax = c.esi - 0x0066fef0u;
        *out = static_cast<uint16_t>(c.eax);
        out += 1;
        c.esi += 0x41cu;
        c.ecx -= 1u;
    } while (c.ecx != 0u);

    // Bubble sort of the 22 words at 0x0071f9a4, restarted while a pass swapped.
    do {
        GP4_GLOBAL(uint8_t, 0x00934df4) = 0;
        int16_t* list = GP4_ARRAY(int16_t, 0x0071f9a4);
        Unknown_0043b689_Car* left = carOfWord(list);
        list += 1;
        for (int step = 0; step < 0x15; ++step) {
            Unknown_0043b689_Car* right = carOfWord(list);
            list += 1;
            const int32_t leftKey = left->Key0F8;
            const int32_t rightKey = right->Key0F8;
            if (!(leftKey > rightKey) && (leftKey < rightKey || left->Unknown66 < right->Unknown66)) {
                const int16_t kept = list[-1];
                list[-1] = list[-2];
                list[-2] = kept;
                GP4_GLOBAL(uint8_t, 0x00934df4) = 0x80;
            }
            left = right;
        }
    } while (GP4_GLOBAL(uint8_t, 0x00934df4) != 0);

    // Rank * 2 into byte +0x415 and the flag bytes into 0x007c31dc...
    const int16_t* ranked = GP4_GLOBAL(const int16_t*, 0x008914f8);
    uint8_t* flagsOut = GP4_ARRAY(uint8_t, 0x007c31dc);
    uint8_t rank = 0;
    for (int index = 0; index < 22; ++index) {
        Unknown_0043b689_Car* car = carOfWord(ranked);
        ranked += 1;
        car->Order415 = rank;
        rank = static_cast<uint8_t>(rank + 2u);
        *flagsOut = car->Flags7C;
        flagsOut += 1;
    }

    Unknown_0043b689_Car* player = carAt(GP4_GLOBAL(uint32_t, 0x00679d30));
    const uint8_t flags = static_cast<uint8_t>(player->Flags88 | 2u);
    player->Flags88 = flags;
    r->eflags = (r->eflags & ~0x8d5u) | orFlags8(flags);
}
GP4_IMPL(0x0043b689, FUN_0043b689, "regs(eax:u32, edx:u32) -> (cf, pf, af, zf, sf, of); globals=layout=cars_0066fef0+lap_0043b482+state_0044334d+state_0040dfe9")
