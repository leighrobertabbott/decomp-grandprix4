#include <gp4/gp4.h>
#include <gp4/flags.h>

namespace f = gp4::flags;

// `test dword [0x66d83c], -1; je skip; push eax; mov eax, [0x66d83c]; neg eax; add eax, [0x66d814]; pop eax;
// js skip`: true when the timer [0x66d83c] is set and the clock [0x66d814] has reached it.
static inline bool TimerExpired(uint32_t& flags) {
    const uint32_t deadline = GP4_GLOBAL(uint32_t, 0x0066d83c);
    flags = f::logic32(deadline);
    if (deadline == 0u) {
        return false;
    }
    const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
    flags = f::add32(0u - deadline, clock);
    return ((clock - deadline) & 0x80000000u) == 0u;
}

// `test byte [0x7aafd4], 0xff; je ok; test byte [0x88f63a], 0xff; je ok; cmp word [0x866e8c], 1; jne skip`.
static inline bool LinkAllows(uint32_t& flags) {
    const uint8_t linked = GP4_GLOBAL(uint8_t, 0x007aafd4);
    flags = f::logic8(linked);
    if (linked == 0u) {
        return true;
    }
    const uint8_t session = GP4_GLOBAL(uint8_t, 0x0088f63a);
    flags = f::logic8(session);
    if (session == 0u) {
        return true;
    }
    const uint16_t peers = GP4_GLOBAL(uint16_t, 0x00866e8c);
    flags = f::sub16(peers, 1u);
    return peers == 1u;
}

// `push [0x66d814]; pop [0x66d83c]; add [0x66d83c], delay`.
static inline void StartTimer(uint32_t delay) {
    GP4_GLOBAL(uint32_t, 0x0066d83c) = GP4_GLOBAL(uint32_t, 0x0066d814);
    GP4_GLOBAL(uint32_t, 0x0066d83c) += delay;
}

// FUNCTION: GP4 0x004425e0
// Timed state machine on the word [0x66d898] (timer [0x66d83c] against the clock [0x66d814]); does nothing
// when byte [0x7c2e6c] is set. State 0: unless byte [0x66d8aa] is negative or the car [0x66d820] has neither
// bit 7 nor bit 6 of +0x7a (or the link [0x7aafd4]/[0x88f63a] is up with [0x866e8c] != 1), 0x00442dc8 runs
// for that car, then 0x00442416 picks the next car: none -> 0x00442c82; the key [0x66d896] -> 0x00442c82;
// another -> state 4 in 2000 ms. With the timer expired in state 0 the pick gives 0x00442c82 / 0x00442cb4 /
// state 1 in 15000 ms. State 1 (and negative states): 0x00442cf5, then on expiry 0x00442cea and state 3 in
// 5000 ms. States 2..3: 0x0051f060 and 0x00442d1f, on expiry (link permitting) 0x00442d50 and state 4.
// State 4: on expiry 0x0051f2a9 with the timer at 500 ms and [0x88f5ec] = clock + 1000, state 5. State 5:
// 0x00442e04 and state 6; state 6: state 7 (5000 ms each). State 7: 0x0051f07a, 0x00442d1f and on expiry
// (link permitting) 0x00442e1f: CF set re-enters the state-0 expiry pick, else 0x00442cdf, 0x004424e2 and
// state 0; states 8 and above go straight to 0. Bit 2 of [0x852d64] is cleared unless the state is 3 or 6.
// One register block is threaded through the callees; the flags are those of the final compare / and.
void __cdecl FUN_004425e0(gp4::Regs* r) {
    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    uint32_t flags = r->eflags;
    uint16_t state = 0;

    const uint8_t frozen = GP4_GLOBAL(uint8_t, 0x007c2e6c);
    flags = f::sub8(frozen, 0u);
    if (frozen != 0u) {
        r->eflags = f::merge(r->eflags, flags);
        return;
    }

    state = GP4_GLOBAL(uint16_t, 0x0066d898);
    s.ecx = (s.ecx & 0xffff0000u) | state;
    flags = f::logic16(state);
    if (state == 0u) {
        goto idle;
    }
    flags = f::sub16(state, 2u);
    if (static_cast<int16_t>(state) < 2) {
        goto state1;
    }
    if (state == 2u) {
        goto state2;
    }
    flags = f::sub16(state, 4u);
    if (static_cast<int16_t>(state) < 4) {
        goto state2;
    }
    if (state == 4u) {
        goto state4;
    }
    flags = f::sub16(state, 6u);
    if (static_cast<int16_t>(state) < 6) {
        goto state5;
    }
    if (state == 6u) {
        goto state6;
    }
    flags = f::sub16(state, 8u);
    if (static_cast<int16_t>(state) < 8) {
        goto state7;
    }
    goto reset;

idle:
    if (TimerExpired(flags)) {
        goto expiredPick;
    }
    flags = f::logic8(GP4_GLOBAL(uint8_t, 0x0066d8aa));
    if ((GP4_GLOBAL(uint8_t, 0x0066d8aa) & 0x80u) != 0u) {
        goto finish;
    }
    s.eax = GP4_GLOBAL(uint32_t, 0x0066d820);
    flags = f::logic32(s.eax);
    if (s.eax != 0u) {
        s.esi = s.eax;
        const uint8_t carFlags = GP4_FIELD(uint8_t, s.esi, 0x7a);
        flags = f::logic8(carFlags & 0x80u);
        if ((carFlags & 0x80u) == 0u) {
            flags = f::logic8(carFlags & 0x40u);
            if ((carFlags & 0x40u) == 0u) {
                goto finish;
            }
        }
        if (!LinkAllows(flags)) {
            goto finish;
        }
        s.eflags = f::merge(s.eflags, flags);
        gp4::call_regs(0x00442dc8, s);
        flags = s.eflags;
    }
    s.eflags = f::merge(s.eflags, flags);
    gp4::call_regs(0x00442416, s);
    flags = s.eflags;
    if ((flags & f::CF) != 0u) {
        s.eflags = f::merge(s.eflags, flags);
        gp4::call_regs(0x00442c82, s);
        flags = s.eflags;
        goto finish;
    }
    flags = f::sub8(static_cast<uint8_t>(s.eax), GP4_GLOBAL(uint8_t, 0x0066d896));
    if (static_cast<uint8_t>(s.eax) == GP4_GLOBAL(uint8_t, 0x0066d896)) {
        s.eflags = f::merge(s.eflags, flags);
        gp4::call_regs(0x00442c82, s);
        flags = s.eflags;
        goto finish;
    }
    s.ecx = 4u;
    StartTimer(0x7d0u);
    goto store;

expiredPick:
    s.eflags = f::merge(s.eflags, flags);
    gp4::call_regs(0x00442416, s);
    flags = s.eflags;
    if ((flags & f::CF) != 0u) {
        s.eflags = f::merge(s.eflags, flags);
        gp4::call_regs(0x00442c82, s);
        flags = s.eflags;
        goto finish;
    }
    flags = f::sub8(static_cast<uint8_t>(s.eax), GP4_GLOBAL(uint8_t, 0x0066d896));
    if (static_cast<uint8_t>(s.eax) == GP4_GLOBAL(uint8_t, 0x0066d896)) {
        s.eflags = f::merge(s.eflags, flags);
        gp4::call_regs(0x00442cb4, s);
        flags = s.eflags;
        goto finish;
    }
    StartTimer(0x3a98u);
    s.ecx = 1u;
    goto store;

state1:
    s.eflags = f::merge(s.eflags, flags);
    gp4::call_regs(0x00442cf5, s);
    flags = s.eflags;
    if (!TimerExpired(flags)) {
        goto finish;
    }
    s.eflags = f::merge(s.eflags, flags);
    gp4::call_regs(0x00442cea, s);
    flags = s.eflags;
    StartTimer(0x1388u);
    s.ecx = 3u;
    goto store;

state2:
    s.eflags = f::merge(s.eflags, flags);
    gp4::call_regs(0x0051f060, s);
    flags = s.eflags;
    s.eflags = f::merge(s.eflags, flags);
    gp4::call_regs(0x00442d1f, s);
    flags = s.eflags;
    if (!TimerExpired(flags)) {
        goto finish;
    }
    if (!LinkAllows(flags)) {
        goto finish;
    }
    s.eflags = f::merge(s.eflags, flags);
    gp4::call_regs(0x00442d50, s);
    flags = s.eflags;
    StartTimer(0x1388u);
    s.ecx = 4u;
    goto store;

state4:
    if (!TimerExpired(flags)) {
        goto finish;
    }
    StartTimer(0x1f4u);
    s.eflags = f::merge(s.eflags, flags);
    gp4::call_regs(0x0051f2a9, s);
    flags = s.eflags;
    GP4_GLOBAL(uint32_t, 0x0088f5ec) = GP4_GLOBAL(uint32_t, 0x0066d814);
    GP4_GLOBAL(uint32_t, 0x0088f5ec) += 0x3e8u;
    s.ecx = 5u;
    goto store;

state5:
    if (!TimerExpired(flags)) {
        goto finish;
    }
    s.eflags = f::merge(s.eflags, flags);
    gp4::call_regs(0x00442e04, s);
    flags = s.eflags;
    StartTimer(0x1388u);
    s.ecx = 6u;
    goto store;

state6:
    if (!TimerExpired(flags)) {
        goto finish;
    }
    StartTimer(0x1388u);
    s.ecx = 7u;
    goto store;

state7:
    s.eflags = f::merge(s.eflags, flags);
    gp4::call_regs(0x0051f07a, s);
    flags = s.eflags;
    s.eflags = f::merge(s.eflags, flags);
    gp4::call_regs(0x00442d1f, s);
    flags = s.eflags;
    if (!TimerExpired(flags)) {
        goto finish;
    }
    if (!LinkAllows(flags)) {
        goto finish;
    }
    s.eflags = f::merge(s.eflags, flags);
    gp4::call_regs(0x00442e1f, s);
    flags = s.eflags;
    if ((flags & f::CF) != 0u) {
        goto expiredPick;
    }
    s.eflags = f::merge(s.eflags, flags);
    gp4::call_regs(0x00442cdf, s);
    flags = s.eflags;
    s.eflags = f::merge(s.eflags, flags);
    gp4::call_regs(0x004424e2, s);
    flags = s.eflags;

reset:
    s.ecx = 0u;

store:
    GP4_GLOBAL(uint16_t, 0x0066d898) = static_cast<uint16_t>(s.ecx);

finish:
    {
        const uint16_t now = GP4_GLOBAL(uint16_t, 0x0066d898);
        flags = f::sub16(now, 3u);
        if (now != 3u) {
            flags = f::sub16(now, 6u);
            if (now != 6u) {
                const uint32_t bits = GP4_GLOBAL(uint32_t, 0x00852d64) & 0xfffffffbu;
                GP4_GLOBAL(uint32_t, 0x00852d64) = bits;
                flags = f::logic32(bits);
            }
        }
    }

    r->eax = s.eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->esi = s.esi;
    r->edi = s.edi;
    r->ebp = s.ebp;
    r->eflags = f::merge(r->eflags, flags);
}
// Spec: 0x00442e1f's composition (state 7) plus the layouts of the other callees (0x00442d50, 0x004424e2,
// 0x00442c82, 0x0051f060, 0x0051f2a9, 0x00442dc8, 0x00442d1f, 0x00442416); gfields draw the state word
// (every case of the dispatch), the timer (zero / expired / pending), the car [0x66d820] (null or a car) and
// the key [0x66d896] (0x81 = a car number byte 0x00442416 can pick from carno_0066ff6c); the lap length word
// [0x868558] (state_004094e3: any i16) is nonzero, as on every track: 0x0043b862 (state 7 -> 0x004424e2 ->
// 0x004424ef) divides by it.
GP4_IMPL(0x004425e0, FUN_004425e0, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:u32, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); cases=600; globals=fuzz:i32+layout=trackgeo_008943a0+tbl_00868598+state_0040d8a5+state_0041104f+flag_007a97a0+cars_0066fef0+state_00403800+div_00403bb6+state_0040f629+lanes_0040da62+pend_0040daeb+state_004094e3+ranges_004322a6+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+state_0040759e+trackfar_008943a0+segflag_004381ab+ctl_007ab590+carst_0040f6d0+flags_00408f7b+carx_0042fe2f+stream_0062d1a0+state_0066d8c0+state_0043b920+carno_0066ff6c+words_0062d210+cars_00443648+state_0040fa01+order_00675958+lap_0043b482+gap_0043530b+scan_00409aec+clock_0066d814+view_00a4df50+dw_00532d48+state_0043260e+state_00442d1f+cur_0066d81c; gfields={0x0066d898:i16{=0,0,1,2,3,4,5,6,7,7,8,9,-1}, 0x0066d83c:u32{=0,1,0x7fffffff,0x80000000,0xffffffff}, 0x0066d8aa:u8{=0,1,0x80}, 0x0066d820:u32{=0,0x66fef0,0x67030c,0x670728}, 0x007aafd4:u8{=0,1}, 0x0088f63a:u8{=0,1}, 0x00866e8c:u16{=0,1,2}, 0x0066d896:u8{=0x81,0x81,0x80,0}, 0x007c2e6c:u8{=0,0,0,1}, 0x00868558:u16[1..0xffff]}")
