#include <gp4/gp4.h>

struct Unknown_00432feb_Car {
    uint8_t Unknown00[0x0e];
    int16_t Unknown0E;
    uint8_t Unknown10[0x18 - 0x10];
    int16_t Unknown18;
    uint8_t Unknown1A[0x7b - 0x1a];
    uint8_t Flags7B;
    uint8_t Unknown7C[0x7e - 0x7c];
    uint8_t Mode7E;
    uint8_t Unknown7F[0x88 - 0x7f];
    uint8_t Flags88;
    uint8_t Unknown89[0x8b - 0x89];
    uint8_t Flags8B;
    uint8_t Unknown8C;
    uint8_t Flags8D;
    uint8_t Unknown8E[0x90 - 0x8e];
    int16_t Unknown90;
    uint8_t Unknown92[0x9a - 0x92];
    int16_t Unknown9A;
    uint8_t Unknown9C[0xca - 0x9c];
    uint8_t FlagsCA;
    uint8_t UnknownCB[0xce - 0xcb];
    uint8_t FlagsCE;
    uint8_t UnknownCF[0xe4 - 0xcf];
    uint8_t StateE4;
    uint8_t FlagsE5;
    uint8_t UnknownE6[0xf0 - 0xe6];
    uint32_t TimerF0;
    uint8_t UnknownF4[0x1ce - 0xf4];
    uint8_t Flags1CE;
    uint8_t Unknown1CF[0x220 - 0x1cf];
    uint32_t Timer220;
    uint8_t Unknown224[0x230 - 0x224];
    uint8_t Flags230;
    uint8_t Unknown231;
    uint8_t Flags232;
    uint8_t Unknown233[0x237 - 0x233];
    uint8_t Unknown237;
    uint8_t Unknown238[0x25f - 0x238];
    uint8_t Flags25F;
    uint8_t Unknown260[0x26b - 0x260];
    uint8_t Flags26B;
};

struct Unknown_00432feb_Seg {
    uint8_t Unknown00[0x1a];
    uint16_t Unknown1A;
};

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of test/and/or/xor on a result of the given width (CF = OF = AF = 0).
static inline uint32_t logicFlags(uint32_t result, uint32_t mask, uint32_t sign) {
    result &= mask;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & sign) != 0u ? 0x80u : 0u);
}
static inline uint32_t logic8(uint32_t v) { return logicFlags(v, 0xffu, 0x80u); }
static inline uint32_t logic16(uint32_t v) { return logicFlags(v, 0xffffu, 0x8000u); }
static inline uint32_t logic32(uint32_t v) { return logicFlags(v, 0xffffffffu, 0x80000000u); }

// Flags of `cmp left, right` / `sub` (and `neg x` as 0 - x) on the given width.
static inline uint32_t subFlags(uint32_t left, uint32_t right, uint32_t mask, uint32_t sign) {
    left &= mask;
    right &= mask;
    const uint32_t result = (left - right) & mask;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result & sign) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & sign) != 0u ? 0x800u : 0u);
}
static inline uint32_t sub8(uint32_t l, uint32_t r) { return subFlags(l, r, 0xffu, 0x80u); }
static inline uint32_t sub16(uint32_t l, uint32_t r) { return subFlags(l, r, 0xffffu, 0x8000u); }
static inline uint32_t sub32(uint32_t l, uint32_t r) { return subFlags(l, r, 0xffffffffu, 0x80000000u); }

// Flags of a 32-bit `add`.
static inline uint32_t add32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((~(left ^ right)) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

static inline void setFlags(gp4::Regs& c, uint32_t flags) {
    c.eflags = (c.eflags & ~0x8d5u) | flags;
}

static inline bool carry(const gp4::Regs& c) { return (c.eflags & gp4::CF) != 0u; }

// The two alternative step routines are called from separate helpers so the compiler cannot
// merge both calls into one call with a selected address.
static __declspec(noinline) void stepFixed(gp4::Regs& c) { gp4::call_regs(0x00432f18, c); }
static __declspec(noinline) void stepOpen(gp4::Regs& c) { gp4::call_regs(0x00432e0e, c); }

// FUNCTION: GP4 0x00432feb
// Per-car state machine on the car record in esi (edi = its track segment), driven by the low
// nibble of byte +0xe4 (states 0..9). Each state runs its step routines through the original
// addresses and may fall through into the next state. Clears byte 0x00622454 first so the first
// 0x00432db9 in this pass runs. The registers and flags are those the last instruction left.
void __cdecl FUN_00432feb(gp4::Regs* r) {
    Unknown_00432feb_Car* car = reinterpret_cast<Unknown_00432feb_Car*>(static_cast<uintptr_t>(r->esi));
    const Unknown_00432feb_Seg* seg = reinterpret_cast<const Unknown_00432feb_Seg*>(static_cast<uintptr_t>(r->edi));

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    GP4_GLOBAL(uint8_t, 0x00622454) = 0;
    c.ecx = seg->Unknown1A & 0x7ffu;
    const uint32_t state = car->StateE4 & 0x0fu;
    c.eax = (c.eax & 0xffffff00u) | state;

    // The dispatch compares al with 0, 9, 1, 2, 3, 4, 5, 8 in turn.
    const bool known = state <= 5u || state == 8u || state == 9u;
    setFlags(c, known ? 0x44u : sub8(state, 8u));

    switch (state) {
    case 0u: {
        car->Flags8D = static_cast<uint8_t>(car->Flags8D | 0x80u);
        if (c.esi == GP4_GLOBAL(uint32_t, 0x00679d2c)) {
            GP4_GLOBAL(uint16_t, 0x0066d888) = 0;
        }
        const uint16_t pos = static_cast<uint16_t>(c.ecx);
        const uint16_t lo = GP4_GLOBAL(uint16_t, 0x00868564);
        setFlags(c, sub16(pos, lo));
        if (static_cast<int16_t>(pos) < static_cast<int16_t>(lo)) {
            goto done;
        }
        const uint16_t hi = GP4_GLOBAL(uint16_t, 0x00868566);
        setFlags(c, sub16(pos, hi));
        if (static_cast<int16_t>(pos) >= static_cast<int16_t>(hi)) {
            car->FlagsCE = static_cast<uint8_t>(car->FlagsCE | 0x20u);
            setFlags(c, logic8(car->FlagsCE));
        }
        car->StateE4 = 1;
        goto done;
    }
    case 1u:
        goto state1;
    case 2u:
        goto state2;
    case 3u:
        goto state3;
    case 4u:
        goto state4;
    case 5u:
        goto state5;
    case 8u:
        goto state8;
    default:
        goto done;
    }

state1:
    setFlags(c, logic8(car->FlagsCE & 0x20u));
    if ((car->FlagsCE & 0x20u) != 0u) {
        goto finish;
    }
    setFlags(c, logic8(car->Flags25F & 1u));
    if ((car->Flags25F & 1u) != 0u) {
        c.eax = 7;
        gp4::call_regs(0x00433ff8, c);
        if (!carry(c)) {
            car->FlagsCA = static_cast<uint8_t>(car->FlagsCA | 2u);
            car->Flags232 = static_cast<uint8_t>(car->Flags232 | 2u);
            setFlags(c, logic8(car->Flags232));
            goto done;
        }
        car->Flags25F = static_cast<uint8_t>(car->Flags25F & 0xfeu);
        car->Flags1CE = static_cast<uint8_t>(car->Flags1CE & 0xfbu);
        car->Flags26B = static_cast<uint8_t>(car->Flags26B | 1u);
        setFlags(c, logic8(car->Flags26B));
        gp4::call_regs(0x00418097, c);
        car->Flags25F = static_cast<uint8_t>(car->Flags25F | 2u);
        setFlags(c, logic8(car->Flags25F));
        car->Mode7E = 4;
        goto done;
    }
    setFlags(c, logic8(car->Flags26B & 1u));
    if ((car->Flags26B & 1u) != 0u) {
        const uint32_t speed = static_cast<uint32_t>(static_cast<int32_t>(car->Unknown90));
        const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00627868);
        c.eax = speed;
        c.edx = speed ^ limit;
        setFlags(c, logic32(c.edx));
        if ((c.edx & 0x80000000u) != 0u) {
            goto done;
        }
        uint32_t bound = limit;
        setFlags(c, logic32(bound));
        if ((bound & 0x80000000u) != 0u) {
            setFlags(c, sub32(0u, bound));
            bound = 0u - bound;
        }
        const uint32_t margin = GP4_GLOBAL(uint32_t, 0x0062d2ac);
        setFlags(c, add32(bound, margin));
        bound += margin;
        c.edx = bound;
        uint32_t magnitude = speed;
        setFlags(c, logic32(magnitude));
        if ((magnitude & 0x80000000u) != 0u) {
            setFlags(c, sub32(0u, magnitude));
            magnitude = 0u - magnitude;
        }
        c.eax = magnitude;
        setFlags(c, sub32(magnitude, bound));
        if (static_cast<int32_t>(magnitude) < static_cast<int32_t>(bound)) {
            goto done;
        }
        goto enter2;
    }

    setFlags(c, logic8(car->FlagsE5 & 4u));
    if ((car->FlagsE5 & 4u) == 0u) {
        const uint16_t level = static_cast<uint16_t>(car->Unknown9A);
        c.edx = (c.edx & 0xffff0000u) | level;
        const uint16_t cap = GP4_GLOBAL(uint16_t, 0x0062a95c);
        setFlags(c, sub16(level, cap));
        if (static_cast<int16_t>(level) >= static_cast<int16_t>(cap)) {
            goto done;
        }
        setFlags(c, sub16(level, 0x1d5u));
        if (static_cast<int16_t>(level) >= 0x1d5) {
            const uint16_t a = static_cast<uint16_t>(car->Unknown18);
            const uint16_t b = static_cast<uint16_t>(car->Unknown0E);
            uint16_t delta = static_cast<uint16_t>(a - b);
            setFlags(c, logic16(delta));
            if ((delta & 0x8000u) != 0u) {
                setFlags(c, sub16(0u, delta));
                delta = static_cast<uint16_t>(0u - delta);
            }
            c.edx = (c.edx & 0xffff0000u) | delta;
            setFlags(c, sub16(delta, 0x4000u));
            if (static_cast<int16_t>(delta) >= 0x4000) {
                goto done;
            }
        }
    }

    setFlags(c, logic8(car->Flags26B & 2u));
    if ((car->Flags26B & 2u) == 0u) {
        uint16_t pace = static_cast<uint16_t>(car->Unknown90);
        const uint8_t direction = GP4_GLOBAL(uint8_t, 0x00868582);
        setFlags(c, logic8(direction));
        if ((direction & 0x80u) != 0u) {
            setFlags(c, sub16(0u, pace));
            pace = static_cast<uint16_t>(0u - pace);
        }
        c.edx = (c.edx & 0xffff0000u) | pace;
        const uint16_t threshold = GP4_GLOBAL(uint16_t, 0x00603b1a);
        setFlags(c, sub16(pace, threshold));
        if (static_cast<int16_t>(pace) < static_cast<int16_t>(threshold)) {
            setFlags(c, logic8(car->FlagsE5 & 4u));
            if ((car->FlagsE5 & 4u) == 0u) {
                goto done;
            }
            setFlags(c, logic8(car->Flags26B & 0x80u));
            if ((car->Flags26B & 0x80u) == 0u) {
                goto done;
            }
            const uint8_t mode = car->Mode7E;
            setFlags(c, sub8(mode, 2u));
            if (mode != 2u) {
                setFlags(c, sub8(mode, 3u));
                if (mode != 3u) {
                    goto done;
                }
            }
            gp4::call_regs(0x00432c21, c);
            setFlags(c, sub32(c.eax, 0u));
            if (static_cast<int32_t>(c.eax) >= 0) {
                goto done;
            }
        }
        car->Flags26B = static_cast<uint8_t>(car->Flags26B | 2u);
        setFlags(c, logic8(car->Flags26B));
        const uint8_t session = GP4_GLOBAL(uint8_t, 0x007c2ea9);
        setFlags(c, sub8(session, 0x80u));
        if (session != 0x80u) {
            c.eax = 0xa;
            gp4::call_regs(0x00434044, c);
        }
    }

    {
        const uint8_t session = GP4_GLOBAL(uint8_t, 0x007c2ea9);
        setFlags(c, sub8(session, 0x80u));
        if (session == 0x80u) {
            stepFixed(c);
        } else {
            stepOpen(c);
        }
        if (!carry(c)) {
            goto done;
        }
    }
    {
        const uint8_t session = GP4_GLOBAL(uint8_t, 0x007c2ea9);
        setFlags(c, sub8(session, 0x80u));
        if (session != 0x80u) {
            const uint8_t mode = car->Mode7E;
            setFlags(c, sub8(mode, 2u));
            bool hit = mode == 2u;
            if (!hit) {
                setFlags(c, sub8(mode, 3u));
                hit = mode == 3u;
            }
            if (!hit) {
                setFlags(c, sub8(mode, 4u));
                hit = mode == 4u;
            }
            if (hit) {
                gp4::call_regs(0x00418097, c);
                car->Flags1CE = static_cast<uint8_t>(car->Flags1CE | 4u);
                setFlags(c, logic8(car->Flags1CE));
            }
        }
    }
    {
        const uint16_t level = static_cast<uint16_t>(car->Unknown9A);
        c.eax = (c.eax & 0xffff0000u) | level;
        setFlags(c, sub16(level, 0x5eu));
        if (static_cast<int16_t>(level) >= 0x5e) {
            car->FlagsCA = static_cast<uint8_t>(car->FlagsCA | 2u);
            car->Flags232 = static_cast<uint8_t>(car->Flags232 | 2u);
            setFlags(c, logic8(car->Flags232));
            goto done;
        }
        car->Flags26B = static_cast<uint8_t>(car->Flags26B & 0xfdu);
        const uint8_t mode = car->Mode7E;
        if (mode == 1u) {
            goto enter2;
        }
        if (mode == 2u || mode == 3u || mode == 4u) {
            setFlags(c, 0x44u);  // the matching `cmp byte [esi+0x7e], k`
            c.eax = 0xd;
            gp4::call_regs(0x00434044, c);
            car->Flags25F = static_cast<uint8_t>(car->Flags25F | 1u);
            car->FlagsCA = static_cast<uint8_t>(car->FlagsCA | 2u);
            car->Flags232 = static_cast<uint8_t>(car->Flags232 | 2u);
            setFlags(c, logic8(car->Flags232));
            goto done;
        }
        goto finish;
    }

enter2:
    car->Flags25F = static_cast<uint8_t>(car->Flags25F & 0xfdu);
    setFlags(c, logic8(car->Flags25F));
    gp4::call_regs(0x0043350c, c);
    gp4::call_regs(0x004327cd, c);
    gp4::call_regs(0x00432777, c);
    car->StateE4 = 2;

state2:
    car->Flags25F = static_cast<uint8_t>(car->Flags25F & 0xf7u);
    setFlags(c, logic8(car->Flags25F));
    gp4::call_regs(0x0043350c, c);
    setFlags(c, logic8(car->Flags25F & 0x10u));
    if ((car->Flags25F & 0x10u) == 0u) {
        gp4::call_regs(0x00432db9, c);
        c.eax = 3;
        gp4::call_regs(0x00433ff8, c);
        if (!carry(c)) {
            goto done;
        }
    }
    car->Flags25F = static_cast<uint8_t>(car->Flags25F & 0xefu);
    car->Flags230 = static_cast<uint8_t>(car->Flags230 | 0x80u);
    setFlags(c, logic8(car->Flags230));
    gp4::call_regs(0x0043283f, c);
    car->StateE4 = 3;

state3:
    gp4::call_regs(0x0043350c, c);
    gp4::call_regs(0x00432db9, c);
    setFlags(c, logic8(car->Flags8B & 0x10u));
    if ((car->Flags8B & 0x10u) != 0u) {
        const uint32_t due = car->Timer220;
        setFlags(c, logic32(due));
        bool reached = true;
        if (due != 0u) {
            const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
            setFlags(c, add32(0u - due, clock));
            reached = ((clock - due) & 0x80000000u) == 0u;
        }
        if (reached) {
            car->Flags8B = static_cast<uint8_t>(car->Flags8B & 0xefu);
            car->Flags88 = static_cast<uint8_t>(car->Flags88 | 0x80u);
            setFlags(c, logic8(car->Flags88));
        }
    }
    setFlags(c, logic8(car->Flags8D & 0x40u));
    if ((car->Flags8D & 0x40u) != 0u) {
        goto done;
    }
    {
        const uint8_t session = GP4_GLOBAL(uint8_t, 0x007c2ea9);
        setFlags(c, sub8(session, 0x80u));
        if (session != 0x80u) {
            const uint32_t due = car->TimerF0;
            setFlags(c, logic32(due));
            if (due == 0u) {
                goto done;
            }
            const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
            setFlags(c, add32(0u - due, clock));
            if (((clock - due) & 0x80000000u) != 0u) {
                goto done;
            }
            goto timed;
        }
    }
    c.eax = 4;
    gp4::call_regs(0x00433ff8, c);
    if (!carry(c)) {
        setFlags(c, logic8(car->Flags8D & 0x20u));
        if ((car->Flags8D & 0x20u) == 0u) {
            setFlags(c, logic8(car->Flags25F & 0x20u));
            if ((car->Flags25F & 0x20u) == 0u) {
                goto done;
            }
            c.eax = 7;
            gp4::call_regs(0x00434044, c);
            car->Flags25F = static_cast<uint8_t>(car->Flags25F & 0xdfu);
            setFlags(c, logic8(car->Flags25F));
            goto done;
        }
        setFlags(c, logic8(car->Flags25F & 0x20u));
        if ((car->Flags25F & 0x20u) != 0u) {
            goto done;
        }
        c.eax = 6;
        gp4::call_regs(0x00434044, c);
        car->Flags25F = static_cast<uint8_t>(car->Flags25F | 0x20u);
        setFlags(c, logic8(car->Flags25F));
        goto done;
    }

timed:
    car->Flags25F = static_cast<uint8_t>(car->Flags25F & 0xdfu);
    setFlags(c, logic8(car->Flags25F));
    {
        const uint8_t session = GP4_GLOBAL(uint8_t, 0x007c2ea9);
        setFlags(c, logic8(session & 0x40u));
        if ((session & 0x40u) == 0u) {
            goto announce;
        }
    }
    {
        const uint32_t start = GP4_GLOBAL(uint32_t, 0x0066d814);
        setFlags(c, add32(start, 0x7d0u));
        car->TimerF0 = start + 0x7d0u;
        car->StateE4 = 4;
    }

state4:
    gp4::call_regs(0x00432db9, c);
    {
        const uint32_t due = car->TimerF0;
        setFlags(c, logic32(due));
        if (due == 0u) {
            goto done;
        }
        const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
        setFlags(c, add32(0u - due, clock));
        if (((clock - due) & 0x80000000u) != 0u) {
            goto done;
        }
        car->Flags7B = static_cast<uint8_t>(car->Flags7B & 0xdfu);
        setFlags(c, logic8(car->Flags7B));
    }

announce:
    {
        const uint8_t session = GP4_GLOBAL(uint8_t, 0x007c2ea9);
        setFlags(c, sub8(session, 0x80u));
        if (session != 0x80u) {
            c.eax = 3;
            gp4::call_regs(0x00434044, c);
        }
    }
    car->StateE4 = 5;
    gp4::call_regs(0x00432a0f, c);
    setFlags(c, logic8(car->Flags7B & 2u));
    if ((car->Flags7B & 2u) == 0u) {
        c.eax = 8;
        gp4::call_regs(0x00434044, c);
    }

state5:
    car->Flags25F = static_cast<uint8_t>(car->Flags25F & 0xf7u);
    setFlags(c, logic8(car->Flags25F));
    gp4::call_regs(0x00432db9, c);
    gp4::call_regs(0x0043350c, c);
    c.eax = 6;
    gp4::call_regs(0x00433ff8, c);
    if (carry(c)) {
        setFlags(c, logic8(car->Flags7B & 2u));
        if ((car->Flags7B & 2u) == 0u) {
            car->Unknown237 = 1;
        }
    }
    c.eax = 5;
    gp4::call_regs(0x00433ff8, c);
    if (!carry(c)) {
        goto done;
    }
    gp4::call_regs(0x00409793, c);
    {
        const uint32_t start = GP4_GLOBAL(uint32_t, 0x0066d814);
        setFlags(c, add32(start, 0x12cu));
        car->TimerF0 = start + 0x12cu;
    }
    gp4::call_regs(0x00432a56, c);
    car->Flags8D = static_cast<uint8_t>(car->Flags8D & 0x7fu);
    setFlags(c, logic8(car->Flags8D));
    car->StateE4 = 6;
    car->StateE4 = 7;
    gp4::call_regs(0x00432a8b, c);

finish:
    car->StateE4 = 8;

state8:
    {
        c.ecx = seg->Unknown1A & 0x7ffu;
        setFlags(c, logic32(c.ecx));
        const uint16_t pos = static_cast<uint16_t>(c.ecx);
        const uint16_t lo = GP4_GLOBAL(uint16_t, 0x00868564);
        setFlags(c, sub16(pos, lo));
        if (static_cast<int16_t>(pos) >= static_cast<int16_t>(lo)) {
            const uint16_t hi = GP4_GLOBAL(uint16_t, 0x00868566);
            setFlags(c, sub16(pos, hi));
            if (static_cast<int16_t>(pos) < static_cast<int16_t>(hi)) {
                goto done;
            }
        }
        car->FlagsCE = static_cast<uint8_t>(car->FlagsCE & 0xdfu);
        setFlags(c, logic8(car->FlagsCE));
        car->StateE4 = 9;
    }

done:
    *r = c;
}
GP4_IMPL(0x00432feb, FUN_00432feb, "regs(eax:u32, edx:u32, ebp:u32, edi:ptr[0x40]:bytes, esi:ptr[0x420]:layout=rec_0043259c+car_004161fb+rec_00432f18+rec_00410894+rec_004087aa+car_00432feb) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); cases=600; globals=fuzz:i32+layout=rows_00434044+state_00432cc2+flag_00622454+state_00432f18+state_004327cd+state_0043260e+state_00432a8b+state_00410894+state_004087aa+state_0066d8c0+counter_0040f0dd+trk_007ad894+state_0040f112+div_0060138c+state_00411109+state_0040efa2")
