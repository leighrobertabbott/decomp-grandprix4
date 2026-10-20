#include <gp4/gp4.h>
#include <gp4/flags.h>

// One helper per callee so the compiler keeps every call address as written.
static __declspec(noinline) void Call514981(gp4::Regs& c) { gp4::call_regs(0x00514981, c); }
static __declspec(noinline) void Call513de3(gp4::Regs& c) { gp4::call_regs(0x00513de3, c); }
static __declspec(noinline) void Call514152(gp4::Regs& c) { gp4::call_regs(0x00514152, c); }
static __declspec(noinline) void Call513f45(gp4::Regs& c) { gp4::call_regs(0x00513f45, c); }
static __declspec(noinline) void Call513fc2(gp4::Regs& c) { gp4::call_regs(0x00513fc2, c); }
static __declspec(noinline) void Call514584(gp4::Regs& c) { gp4::call_regs(0x00514584, c); }

static inline uint8_t* Record(const gp4::Regs& c) {
    return reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.esi));
}

static inline void SetFlags(gp4::Regs& c, uint32_t arith) {
    c.eflags = gp4::flags::merge(c.eflags, arith);
}

// pushal/popal event pass for the viewed car: only when the master byte 0x66d7a4 is clear and esi
// is the car at [0x679d2c]. Runs 0x514981, sets 0x893cf9 = 0xff, then by the state bits of record
// byte +0x82 (bit 1 off -> 0x513de3; +0x8d bit 4 with +0x84 == 7 -> event 0x17 with +0x8d bit 4
// cleared around it; +0x8d bit 4 with +0x82 bit 6 -> 0x513de3 + event 0x17; bit 5 -> 0x513de3 +
// event 0x18; bit 3 -> 0x513de3 then 0x513f45 / 0x513fc2 by bit 4; bit 2 -> 0x514584, and when it
// answers CF set: +0x4c = [0x66d814], bit 2 cleared and the pass repeats; else 0x513de3). Finally
// bit 7 of +0x88 is cleared. popal restores every GPR; the flags are those of the last `and`
// (or of the master/selection test on the early exits).
// FUNCTION: GP4 0x00513ce2
void __cdecl FUN_00513ce2(gp4::Regs* r) {
    const uint8_t master = GP4_GLOBAL(uint8_t, 0x0066d7a4);
    if (master != 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(master));
        return;
    }
    const uint32_t viewed = GP4_GLOBAL(uint32_t, 0x00679d2c);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(r->esi, viewed));
    if (r->esi != viewed) {
        return;
    }

    gp4::Regs c = *r;
    for (;;) {
        Call514981(c);
        GP4_GLOBAL(uint8_t, 0x00893cf9) = 0xffu;

        uint8_t* rec = Record(c);
        SetFlags(c, gp4::flags::logic8(static_cast<uint8_t>(rec[0x82] & 0x02u)));
        if ((rec[0x82] & 0x02u) == 0u) {
            Call513de3(c);
            break;
        }

        SetFlags(c, gp4::flags::logic8(static_cast<uint8_t>(rec[0x8d] & 0x10u)));
        if ((rec[0x8d] & 0x10u) != 0u) {
            SetFlags(c, gp4::flags::sub8(rec[0x84], 7u));
            if (rec[0x84] == 7u) {
                const uint8_t cleared = static_cast<uint8_t>(rec[0x8d] & 0xefu);
                rec[0x8d] = cleared;
                SetFlags(c, gp4::flags::logic8(cleared));
                GP4_GLOBAL(uint32_t, 0x0088ff30) = 0x17u;
                Call514152(c);
                rec = Record(c);
                rec[0x8d] = static_cast<uint8_t>(rec[0x8d] | 0x10u);
                break;
            }
            SetFlags(c, gp4::flags::logic8(static_cast<uint8_t>(rec[0x82] & 0x40u)));
            if ((rec[0x82] & 0x40u) != 0u) {
                Call513de3(c);
                GP4_GLOBAL(uint32_t, 0x0088ff30) = 0x17u;
                Call514152(c);
                break;
            }
        }

        SetFlags(c, gp4::flags::logic8(static_cast<uint8_t>(rec[0x82] & 0x20u)));
        if ((rec[0x82] & 0x20u) != 0u) {
            Call513de3(c);
            GP4_GLOBAL(uint32_t, 0x0088ff30) = 0x18u;
            Call514152(c);
            break;
        }

        SetFlags(c, gp4::flags::logic8(static_cast<uint8_t>(rec[0x82] & 0x08u)));
        if ((rec[0x82] & 0x08u) != 0u) {
            Call513de3(c);
            rec = Record(c);
            SetFlags(c, gp4::flags::logic8(static_cast<uint8_t>(rec[0x82] & 0x10u)));
            if ((rec[0x82] & 0x10u) == 0u) {
                Call513f45(c);
            } else {
                Call513fc2(c);
            }
            break;
        }

        SetFlags(c, gp4::flags::logic8(static_cast<uint8_t>(rec[0x82] & 0x04u)));
        if ((rec[0x82] & 0x04u) == 0u) {
            Call513de3(c);
            break;
        }

        Call514584(c);
        if ((c.eflags & gp4::flags::CF) == 0u) {
            break;
        }
        rec = Record(c);
        const uint32_t now = GP4_GLOBAL(uint32_t, 0x0066d814);
        GP4_FIELD(uint32_t, rec, 0x4c) = now;
        const uint8_t bits = static_cast<uint8_t>(rec[0x82] & 0xfbu);
        rec[0x82] = bits;
        SetFlags(c, gp4::flags::logic8(bits));
    }

    uint8_t* rec = Record(c);
    const uint8_t last = static_cast<uint8_t>(rec[0x88] & 0x7fu);
    rec[0x88] = last;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(last));
}
GP4_IMPL(0x00513ce2, FUN_00513ce2, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, edi:u32, ebp:u32, esi:ptr[0xf0]:u8{7,7,7,0,2,6,0x0a,0x10,0x12,0x16,0x1a,0x22,0x42,0xff}) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=player_0040f8d9+bits_005174cb+links_00521d6f+disp_005141b8+events_00514152+times_00513f45+carno_0066ff6c+cars_004362a9+due_0066ffb0+pair_00514584+state_00432a8b")
