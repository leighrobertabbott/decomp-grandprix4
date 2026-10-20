#include <gp4/gp4.h>
#include <gp4/flags.h>

// Score of the 7-entry candidate-kind weight table 0x6f3a18 (indexed 0..6).
static __declspec(noinline) int32_t KindWeight(uint32_t kind) {
    return GP4_ARRAY(int32_t, 0x006f3a18)[kind];
}

// Calls a register routine with esi = the record it examines; every other register is the
// caller's (the routines read only esi).
static inline gp4::Regs CallOn(uint32_t addr, const gp4::Regs& base, uint32_t esi) {
    gp4::Regs c = base;
    c.esi = esi;
    gp4::call_regs(addr, c);
    return c;
}

// 0x20 - (place - 1), at least 1 (`dec eax; neg eax; add eax,0x20; cmp eax,1; jge`).
static inline int32_t PlaceScore(uint32_t place) {
    int32_t score = static_cast<int32_t>(0x20u - (place - 1u));
    if (score < 1) {
        score = 1;
    }
    return score;
}

struct Pick {
    int32_t score;   // ebx
    uint32_t kind;   // ecx
};

// `cmp ebx, eax; jg skip; mov ebx,eax; mov ecx,kind; mov [0x6f3ab4],who`
static inline void Consider(Pick& pick, int32_t score, uint32_t kind, uint32_t who) {
    if (!(pick.score > score)) {
        pick.score = score;
        pick.kind = kind;
        GP4_GLOBAL(uint32_t, 0x006f3ab4) = who;
    }
}

// pushal routine. Unless the due time [0x6f3aac] is set and not reached yet, walks the
// order list backwards from [0x6043f8] to the first negative word and scores every car
// record (0x66fef0 + word) whose +0x7a bits 0x80/0x40 and +0x8d bit 2 are clear, through
// the car predicates 0x408134/0x408141/0x40814e/0x408170/0x408186/0x4081a1, the place
// 0x408309 and the random byte 0x4015dc. The best candidate (ties go to the later one) is
// kept in [0x6f3ab4] with kind ecx 1..6; if one was found, it is not flagged, not the
// selected car [0x679d30], and either the hold time [0x6f3ab0] has passed or its kind
// outweighs the current kind [0x6f3abc], it is published (0x852d3c, 0x6f3ab8, 0x6f3abc),
// 0x525dc2 is called and the due/hold/expiry times are rearmed from the clock 0x66d814.
// popal restores every GPR; the flags are those of the last flag writer.
// FUNCTION: GP4 0x00525a8c
void __cdecl FUN_00525a8c(gp4::Regs* r) {
    using namespace gp4::flags;
    uint32_t arith;

    const uint32_t due = GP4_GLOBAL(uint32_t, 0x006f3aac);
    if (due != 0u) {
        arith = add32(0u - due, GP4_GLOBAL(uint32_t, 0x0066d814));
        if ((arith & SF) != 0u) {
            r->eflags = merge(r->eflags, arith);
            return;
        }
    }

    GP4_GLOBAL(uint32_t, 0x006f3ab4) = GP4_GLOBAL(uint32_t, 0x00679d30);
    Pick pick = {0, 0xffffffffu};
    const uint16_t* entry = GP4_GLOBAL(const uint16_t*, 0x006043f8);
    for (;;) {
        --entry;
        const uint16_t word = *entry;
        if ((word & 0x8000u) != 0u) {
            break;
        }
        uint8_t* car = GP4_ARRAY(uint8_t, 0x0066fef0) + word;
        const uint32_t carAddr = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car));
        if ((car[0x7a] & 0x80u) != 0u || (car[0x7a] & 0x40u) != 0u || (car[0x8d] & 2u) != 0u) {
            continue;
        }

        gp4::Regs c = CallOn(0x00408309, *r, carAddr);
        if (static_cast<int32_t>(c.eax) <= 3) {
            const int8_t laps = static_cast<int8_t>(car[0x70]);
            bool scored = true;
            uint32_t value = 0u;
            if (GP4_GLOBAL(uint8_t, 0x007c2ea9) == 0x80u && laps > 1) {
                c = CallOn(0x004015dc, *r, carAddr);
                value = c.eax & 0xffu;
            } else if (GP4_GLOBAL(uint8_t, 0x007c2ea9) != 0x80u && laps <= 1) {
                scored = false;
            } else {
                c = CallOn(0x00408309, *r, carAddr);
                value = static_cast<uint32_t>(PlaceScore(c.eax)) >> 6;
            }
            if (scored) {
                Consider(pick, static_cast<int32_t>(value + static_cast<uint32_t>(KindWeight(5))), 5u, carAddr);
            }
        }

        c = CallOn(0x00408134, *r, carAddr);
        if ((c.eflags & CF) != 0u) {
            c = CallOn(0x00408309, *r, carAddr);
            Consider(pick, PlaceScore(c.eax) + KindWeight(2), 2u, carAddr);
        }

        c = CallOn(0x00408141, *r, carAddr);
        if ((c.eflags & CF) != 0u) {
            c = CallOn(0x00408186, *r, carAddr);
            if ((c.eflags & CF) != 0u) {
                const uint32_t partner = c.eax;
                c = CallOn(0x00408309, *r, partner);
                if (static_cast<int32_t>(c.eax) < 5) {
                    c = CallOn(0x004015dc, *r, carAddr);
                    if ((c.eax & 0xffu) >= 0x40u) {
                        Consider(pick, KindWeight(4), 4u, partner);
                    } else {
                        Consider(pick, KindWeight(6), 6u, carAddr);
                    }
                }
            }
        } else {
            c = CallOn(0x0040814e, *r, carAddr);
            if ((c.eflags & CF) != 0u) {
                const int8_t limit = static_cast<int8_t>(static_cast<uint8_t>(car[0x70] * 2u + 7u));
                c = CallOn(0x00408309, *r, carAddr);
                if (static_cast<int8_t>(static_cast<uint8_t>(c.eax - 1u)) <= limit) {
                    c = CallOn(0x004015dc, *r, carAddr);
                    if ((c.eax & 0xffu) >= 0x40u) {
                        c = CallOn(0x00408309, *r, carAddr);
                        Consider(pick, PlaceScore(c.eax) + KindWeight(1), 1u, carAddr);
                    } else {
                        c = CallOn(0x00408170, *r, carAddr);
                        if ((c.eflags & CF) != 0u) {
                            const uint32_t linkedAddr = c.eax;
                            const uint8_t* linked = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(linkedAddr));
                            if ((linked[0x7a] & 0x80u) == 0u) {
                                c = CallOn(0x00408309, *r, linkedAddr);
                                Consider(pick, PlaceScore(c.eax) + KindWeight(6), 6u, linkedAddr);
                            }
                        }
                    }
                }
            }
        }

        c = CallOn(0x004081a1, *r, carAddr);
        if ((c.eflags & CF) != 0u) {
            Consider(pick, KindWeight(3), 3u, carAddr);
        }
    }

    arith = sub32(pick.kind, 0xffffffffu);
    if (pick.kind != 0xffffffffu) {
        const uint32_t bestAddr = GP4_GLOBAL(uint32_t, 0x006f3ab4);
        const uint8_t* best = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(bestAddr));
        arith = logic8(best[0x7a] & 0x80u);
        if ((best[0x7a] & 0x80u) == 0u) {
            const uint32_t selected = GP4_GLOBAL(uint32_t, 0x00679d30);
            arith = sub32(bestAddr, selected);
            if (bestAddr != selected) {
                bool publish = true;
                const uint32_t hold = GP4_GLOBAL(uint32_t, 0x006f3ab0);
                if (hold != 0u) {
                    arith = add32(0u - hold, GP4_GLOBAL(uint32_t, 0x0066d814));
                    if ((arith & SF) != 0u) {
                        const int32_t current = KindWeight(GP4_GLOBAL(uint32_t, 0x006f3abc));
                        const int32_t candidate = KindWeight(pick.kind);
                        arith = sub32(static_cast<uint32_t>(current), static_cast<uint32_t>(candidate));
                        publish = current < candidate;
                    }
                }
                if (publish) {
                    GP4_GLOBAL(uint32_t, 0x00852d3c) = bestAddr;
                    GP4_GLOBAL(int32_t, 0x006f3ab8) = pick.score;
                    GP4_GLOBAL(uint32_t, 0x006f3abc) = pick.kind;
                    gp4::Regs c = *r;
                    c.ebx = static_cast<uint32_t>(pick.score);
                    c.ecx = pick.kind;
                    c.esi = bestAddr;
                    gp4::call_regs(0x00525dc2, c);
                    const uint32_t kind = pick.kind;
                    GP4_GLOBAL(uint32_t, 0x006f3aac) = GP4_GLOBAL(uint32_t, 0x0066d814);
                    GP4_GLOBAL(uint32_t, 0x006f3aac) += GP4_ARRAY(uint32_t, 0x006f39c4)[kind];
                    uint32_t holdFor = GP4_ARRAY(uint32_t, 0x006f39e0)[kind];
                    if ((GP4_GLOBAL(uint8_t, 0x00852d5c) & 1u) != 0u) {
                        holdFor = GP4_ARRAY(uint32_t, 0x006f39fc)[kind];
                    }
                    GP4_GLOBAL(uint32_t, 0x006f3ab0) = GP4_GLOBAL(uint32_t, 0x0066d814);
                    GP4_GLOBAL(uint32_t, 0x006f3ab0) += holdFor;
                    const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
                    GP4_GLOBAL(uint32_t, 0x00852d48) = clock + 0x7d0u;
                    arith = add32(clock, 0x7d0u);
                }
            }
        }
    }
    r->eflags = merge(r->eflags, arith);
}
GP4_IMPL(0x00525a8c, FUN_00525a8c, "regs(esi:rec[0x66fef0,0x41c,0..21]) -> (cf, pf, af, zf, sf, of); globals=layout=carbg_0066fef0+mode_00430c8e+cars_0066fef0+cars_004362a9+list_0066fe3c+cursor_006043fc+save_00510309+order_00418f8f+carno_0066ff6c+clock_0066d814+state_0062d1b4")
