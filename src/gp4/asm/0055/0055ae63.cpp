#include <gp4/gp4.h>
#include <gp4/flags.h>

// One 0x2c-byte channel record. Two banks of 16 records: 0x88eff2 and 0x88f2b2 (= +0x2c0).
#pragma pack(push, 1)
struct Unknown_88eff2 {
    uint16_t Unknown00;   // channel id: 0xffff = unused, else bit number in the bit set 0x8705b8
    uint16_t Unknown02;   // bits OR'd into the words 0x8705d4/d8/dc/e0
    uint16_t Unknown04;   // phase 1 length (copied to +0x24)
    uint16_t Unknown06;   // phase 2 length: random mask
    uint16_t Unknown08;   // phase 2 length: base
    uint16_t Unknown0a;   // phase 3 length: random mask
    uint16_t Unknown0c;   // phase 3 length: base
    uint16_t Unknown0e;   // phase 4 length: random mask
    uint16_t Unknown10;   // phase 4 length: base
    uint16_t Unknown12;   // phase 5 length: random mask
    uint16_t Unknown14;   // phase 5 length: base
    uint16_t Unknown16;   // cx for 0x0055ad9b entering phase 2
    uint16_t Unknown18;   // dx for 0x0055ad9b entering phase 2
    uint16_t Unknown1a;   // cx for 0x0055ad9b entering phase 4
    uint16_t Unknown1c;   // dx for 0x0055ad9b entering phase 4
    uint16_t Unknown1e;   // reload of +0x22
    uint16_t Unknown20;   // phase 2/4 timer
    uint16_t Unknown22;   // sub-period timer
    uint16_t Unknown24;   // phase 1 timer
    uint16_t Unknown26;   // phase 3/5 timer
    uint16_t Unknown28;   // reload of +0x2a
    uint16_t Unknown2a;   // second sub-period timer
};
#pragma pack(pop)

typedef Unknown_88eff2 Rec;

// MSVC folds `0x88f2b2 - 0x88eff2` to a constant; the original computes it with both addresses.
static __declspec(noinline) uint32_t Span(uint32_t end, uint32_t start) {
    return end - start;
}

// `cmp dword [0x852d28], 0x8b2a60; jb`
static __declspec(noinline) bool AtOrAbove(uint32_t value, uint32_t limit) {
    return value >= limit;
}

static inline void setLow16(uint32_t& reg, uint32_t value) {
    reg = (reg & 0xffff0000u) | (value & 0xffffu);
}

static inline Rec* RecAt(uint32_t address) {
    return reinterpret_cast<Rec*>(static_cast<uintptr_t>(address));
}

// Phase length: (draw of 0x004015dc & mask) + base, or just base when the mask is zero. ax only.
static inline uint16_t DrawLength(gp4::Regs& s, const uint16_t* mask, const uint16_t* base) {
    if (*mask != 0u) {
        gp4::call_regs(0x004015dc, s);
        setLow16(s.eax, s.eax & *mask);
        setLow16(s.eax, s.eax + *base);
    } else {
        setLow16(s.eax, *base);
    }
    return static_cast<uint16_t>(s.eax);
}

// Entering phase 2 or 4 with a nonzero length: raise all four masks, reload both sub-timers,
// then 0x0055ae30 and 0x0055ad9b (cx/dx from the record).
static inline void StartOn(gp4::Regs& s, Rec* rec, const uint16_t* cxField, const uint16_t* dxField) {
    GP4_GLOBAL(uint16_t, 0x008705d4) |= rec->Unknown02;
    GP4_GLOBAL(uint16_t, 0x008705d8) |= rec->Unknown02;
    GP4_GLOBAL(uint16_t, 0x008705dc) |= rec->Unknown02;
    GP4_GLOBAL(uint16_t, 0x008705e0) |= rec->Unknown02;
    setLow16(s.eax, rec->Unknown02);
    rec->Unknown22 = rec->Unknown1e;
    rec->Unknown2a = rec->Unknown28;
    s.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(rec));
    gp4::call_regs(0x0055ae30, s);
    setLow16(s.ecx, *cxField);
    setLow16(s.edx, *dxField);
    gp4::call_regs(0x0055ad9b, s);
}

// One tick of phase 2 or 4 (state word already set to `phase`).
static inline void TickOn(gp4::Regs& s, Rec* rec, uint16_t* state) {
    rec->Unknown20 = static_cast<uint16_t>(rec->Unknown20 - 1u);
    if (rec->Unknown20 == 0u) {
        GP4_GLOBAL(uint16_t, 0x008705d8) |= rec->Unknown02;
        GP4_GLOBAL(uint16_t, 0x008705dc) |= rec->Unknown02;
        GP4_GLOBAL(uint16_t, 0x008705e0) |= rec->Unknown02;
        *state = static_cast<uint16_t>(*state + 1u);
    } else {
        GP4_GLOBAL(uint16_t, 0x008705d4) |= rec->Unknown02;
        rec->Unknown22 = static_cast<uint16_t>(rec->Unknown22 - 1u);
        if (rec->Unknown22 == 0u) {
            GP4_GLOBAL(uint16_t, 0x008705d8) |= rec->Unknown02;
            rec->Unknown22 = rec->Unknown1e;
            rec->Unknown2a = static_cast<uint16_t>(rec->Unknown2a - 1u);
            if (rec->Unknown2a == 0u) {
                GP4_GLOBAL(uint16_t, 0x008705e0) |= rec->Unknown02;
                rec->Unknown2a = rec->Unknown28;
            }
        }
    }
    setLow16(s.eax, rec->Unknown02);
}

// FUNCTION: GP4 0x0055ae63
// pushal/popal wrapper over 16 channels (state words 0x874b72[i], bank offsets 0x874b92[i] = 0 or
// 0x2c0, records 0x88eff2 + 0x2c*i + bank). A channel whose id is not 0xffff and whose bit is set
// in 0x8705b8 steps a 6-state machine: 0 init (pick the bank, load the phase 1 timer), 1 delay,
// 2 on (random length, masks raised), 3 delay, 4 on, 5 delay, then back to 2. The accumulated
// masks 0x8705d8/d4/dc/e0 are published to 0x874b40/0x8705f0/0x874b44/0x874b48 and cleared.
// All GPRs are restored; the flags are those of the final `sub word [0x934df0], 1`.
void __cdecl FUN_0055ae63(gp4::Regs* r) {
    gp4::Regs s;
    uint16_t* state;
    Rec* rec;
    uint16_t type;
    uint16_t count;
    uint32_t arith;

    GP4_GLOBAL(uint32_t, 0x00852d28) = r->edi;
    s = *r;
    s.st_in = 0;
    s.st_out = 0;
    s.ebx = 0x00874b72u;
    state = reinterpret_cast<uint16_t*>(static_cast<uintptr_t>(s.ebx));
    GP4_GLOBAL(uint32_t, 0x0088f5c4) = 0;
    GP4_GLOBAL(uint16_t, 0x00934df0) = 0xf;

    do {
        s.eax = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(state[0x10])));
        rec = RecAt(0x0088eff2u + GP4_GLOBAL(uint32_t, 0x0088f5c4) + s.eax);
        setLow16(s.edx, 0);
        type = rec->Unknown00;
        setLow16(s.eax, type);
        if (type == 0xffffu) {
            goto next;
        }
        if (type == 3u && AtOrAbove(GP4_GLOBAL(uint32_t, 0x00852d28), 0x008b2a60u)) {
            // pushal; mov ax, [edi+0x20]; popal: no effect
        }
        {
            const int32_t bit = static_cast<int16_t>(type);
            const uint16_t* bits = GP4_ARRAY(uint16_t, 0x008705b8);
            if (((bits[bit >> 4] >> (bit & 15)) & 1u) == 0u) {
                goto next;
            }
        }
        switch (*state) {
        case 1: goto tick1;
        case 2: goto tick2;
        case 3: goto tick3;
        case 4: goto tick4;
        case 5: goto tick5;
        default: goto init;
        }

    init:
        rec = RecAt(0x0088eff2u + GP4_GLOBAL(uint32_t, 0x0088f5c4));
        state[0x10] = 0;
        if (!((rec->Unknown00 == 3u || rec->Unknown00 == 2u) && (GP4_GLOBAL(uint8_t, 0x008705ec) & 4u) != 0u)
            && GP4_GLOBAL(uint32_t, 0x008705cc) != 0u) {
            s.eax = Span(0x0088f2b2u, 0x0088eff2u);
            state[0x10] = static_cast<uint16_t>(s.eax);
        }
        s.eax = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(state[0x10])));
        rec = RecAt(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(rec)) + s.eax);
        *state = 1;
        rec->Unknown24 = rec->Unknown04;
        if (rec->Unknown04 == 0u) {
            *state = 2;
        }
        if (*state == 1u) {
            goto next;
        }
        goto enter2;

    tick1:
        rec->Unknown24 = static_cast<uint16_t>(rec->Unknown24 - 1u);
        if (rec->Unknown24 == 0u) {
            *state = 2;
        }
        if (*state == 1u) {
            goto next;
        }
        // fall through

    enter2:
        *state = 2;
        rec->Unknown20 = DrawLength(s, &rec->Unknown06, &rec->Unknown08);
        if (static_cast<uint16_t>(s.eax) == 0u) {
            *state = static_cast<uint16_t>(*state + 1u);
        } else {
            StartOn(s, rec, &rec->Unknown16, &rec->Unknown18);
        }
        if (*state == 2u) {
            goto next;
        }
        goto enter3;

    tick2:
        *state = 2;
        TickOn(s, rec, state);
        if (*state == 2u) {
            goto next;
        }
        // fall through

    enter3:
        *state = 3;
        rec->Unknown26 = DrawLength(s, &rec->Unknown0a, &rec->Unknown0c);
        if (static_cast<uint16_t>(s.eax) == 0u) {
            *state = static_cast<uint16_t>(*state + 1u);
        }
        if (*state == 3u) {
            goto next;
        }
        goto enter4;

    tick3:
        rec->Unknown26 = static_cast<uint16_t>(rec->Unknown26 - 1u);
        if (rec->Unknown26 == 0u) {
            *state = static_cast<uint16_t>(*state + 1u);
        }
        if (*state == 3u) {
            goto next;
        }
        // fall through

    enter4:
        *state = 4;
        rec->Unknown20 = DrawLength(s, &rec->Unknown0e, &rec->Unknown10);
        if (static_cast<uint16_t>(s.eax) == 0u) {
            *state = static_cast<uint16_t>(*state + 1u);
        } else {
            StartOn(s, rec, &rec->Unknown1a, &rec->Unknown1c);
        }
        if (*state == 4u) {
            goto next;
        }
        goto enter5;

    tick4:
        *state = 4;
        TickOn(s, rec, state);
        if (*state == 4u) {
            goto next;
        }
        // fall through

    enter5:
        *state = 5;
        rec->Unknown26 = DrawLength(s, &rec->Unknown12, &rec->Unknown14);
        if (static_cast<uint16_t>(s.eax) == 0u) {
            *state = static_cast<uint16_t>(*state + 1u);
        }
        if (*state != 5u) {
            goto enter2;
        }
        goto next;

    tick5:
        rec->Unknown26 = static_cast<uint16_t>(rec->Unknown26 - 1u);
        if (rec->Unknown26 == 0u) {
            *state = static_cast<uint16_t>(*state + 1u);
        }
        if (*state != 5u) {
            goto enter2;
        }

    next:
        state += 1;
        s.ebx += 2u;
        GP4_GLOBAL(uint32_t, 0x0088f5c4) += 0x2cu;
        count = GP4_GLOBAL(uint16_t, 0x00934df0);
        arith = gp4::flags::sub16(count, 1u);
        GP4_GLOBAL(uint16_t, 0x00934df0) = static_cast<uint16_t>(count - 1u);
    } while (static_cast<int16_t>(count - 1u) >= 0);

    GP4_GLOBAL(uint32_t, 0x00874b40) = GP4_GLOBAL(uint32_t, 0x008705d8);
    GP4_GLOBAL(uint32_t, 0x008705f0) = GP4_GLOBAL(uint32_t, 0x008705d4);
    GP4_GLOBAL(uint32_t, 0x00874b44) = GP4_GLOBAL(uint32_t, 0x008705dc);
    GP4_GLOBAL(uint32_t, 0x00874b48) = GP4_GLOBAL(uint32_t, 0x008705e0);
    GP4_GLOBAL(uint32_t, 0x008705d8) = 0;
    GP4_GLOBAL(uint32_t, 0x008705d4) = 0;
    GP4_GLOBAL(uint32_t, 0x008705dc) = 0;
    GP4_GLOBAL(uint32_t, 0x008705e0) = 0;

    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0055ae63, FUN_0055ae63, "regs(eax:u32, ecx:u32, edx:u32, edi:u32{0x8b2a60,0x8b2a5c,0x8b2b00}) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=seed_0062d1b4+nib_00870618+chan_0055ae63")
