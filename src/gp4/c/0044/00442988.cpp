#include <gp4/gp4.h>
#include <gp4/flags.h>

static inline void setFlags(gp4::Regs* r, uint32_t arith) {
    r->eflags = gp4::flags::merge(r->eflags, arith);
}

static inline bool carry(const gp4::Regs* r) {
    return (r->eflags & gp4::flags::CF) != 0u;
}

// FUNCTION: GP4 0x00442988
// Timed state machine over the state word [0x66d898] and the due time [0x66d83c] (compared with
// the race clock [0x66d814]); register results are those of the callees, the flags those of
// the last flag-writing instruction on each exit path.
void __cdecl FUN_00442988(gp4::Regs* r) {
    using namespace gp4::flags;
    uint32_t& due = GP4_GLOBAL(uint32_t, 0x0066d83c);
    uint16_t& state = GP4_GLOBAL(uint16_t, 0x0066d898);
    uint16_t& counter = GP4_GLOBAL(uint16_t, 0x0066d894);

    if (GP4_GLOBAL(uint8_t, 0x007c2e75) != 0u) {
        gp4::call_regs(0x0044330d, *r);
        if (!carry(r)) {
            return;
        }
        const uint32_t limit = static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x007ad89c))
            * static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x007c2eb8));
        const uint32_t next = static_cast<uint32_t>(counter) + 1u;
        r->eax = limit;
        r->edx = next;
        const uint32_t compared = sub32(next, limit);
        if (static_cast<int32_t>(next) < static_cast<int32_t>(limit)) {
            gp4::call_regs(0x00442468, *r);
            if (carry(r)) {
                return;
            }
            goto release;
        }
        if (static_cast<int32_t>(next) > static_cast<int32_t>(limit)) {
            setFlags(r, compared);
            return;
        }
        setFlags(r, inc16(counter, compared));
        counter = static_cast<uint16_t>(counter + 1u);
        return;
    }

    {
        const uint16_t current = state;
        r->ecx = (r->ecx & 0xffff0000u) | current;

        if (current == 1u) {
            // 0x442ae7
            gp4::call_regs(0x00442cf5, *r);
            if (due == 0u) {
                setFlags(r, logic32(0u));
                return;
            }
            {
                const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
                setFlags(r, add32(0u - due, clock));
                if (static_cast<int32_t>(clock - due) < 0) {
                    return;
                }
                due = clock + 0x1f4u;
            }
            gp4::call_regs(0x0051f2a9, *r);
            {
                const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
                GP4_GLOBAL(uint32_t, 0x0088f5ec) = clock + 0x3e8u;
                setFlags(r, add32(clock, 0x3e8u));
            }
            r->ecx = (r->ecx & 0xffff0000u) | 2u;
            state = 2u;
            return;
        }

        if (static_cast<int16_t>(current) > 1) {
            // 0x442b48
            gp4::call_regs(0x00442cf5, *r);
            if (due == 0u) {
                setFlags(r, logic32(0u));
                return;
            }
            const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
            setFlags(r, add32(0u - due, clock));
            if (static_cast<int32_t>(clock - due) < 0) {
                return;
            }
            goto release;
        }

        // state below 1
        if (due != 0u) {
            const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
            if (static_cast<int32_t>(clock - due) >= 0) {
                // 0x442aad: due
                if (GP4_GLOBAL(int16_t, 0x007c2eb8) < 2) {
                    gp4::call_regs(0x00442cb4, *r);
                    return;
                }
                gp4::call_regs(0x00442468, *r);
                const uint32_t now = GP4_GLOBAL(uint32_t, 0x0066d814);
                due = now + 0x3a98u;
                setFlags(r, add32(now, 0x3a98u));
                r->ecx = 1u;
                state = 1u;
                return;
            }
        }

        // 0x442a1a
        gp4::call_regs(0x0051f184, *r);
        if (carry(r)) {
            gp4::call_regs(0x00442468, *r);
            if (carry(r)) {
                return;
            }
            goto release;
        }

        r->eax = GP4_GLOBAL(uint32_t, 0x0066d820);
        if (r->eax != 0u) {
            r->esi = r->eax;
            const uint8_t flags7a = GP4_FIELD(uint8_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi)), 0x7a);
            if ((flags7a & 0x80u) == 0u && (flags7a & 0x40u) == 0u) {
                setFlags(r, logic8(0u));
                return;
            }
            if (GP4_GLOBAL(uint8_t, 0x007aafd4) != 0u && GP4_GLOBAL(uint8_t, 0x0088f63a) != 0u) {
                const uint16_t mode = GP4_GLOBAL(uint16_t, 0x00866e8c);
                if (mode != 1u) {
                    setFlags(r, sub16(mode, 1u));
                    return;
                }
            }
            gp4::call_regs(0x00442dc8, *r);
        }

        // 0x442a71
        gp4::call_regs(0x00442468, *r);
        if (carry(r)) {
            return;
        }
        {
            const uint8_t wanted = GP4_GLOBAL(uint8_t, 0x0066d897);
            r->eax = (r->eax & 0xffffff00u) | wanted;
            if (wanted == GP4_GLOBAL(uint8_t, 0x0066d896)) {
                goto release;
            }
            const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
            due = clock + 0x7d0u;
            setFlags(r, add32(clock, 0x7d0u));
            r->ecx = 1u;
            state = 1u;
            return;
        }
    }

release:
    // 0x442b6a
    if (GP4_GLOBAL(uint8_t, 0x007aafd4) != 0u && GP4_GLOBAL(uint8_t, 0x0088f63a) != 0u) {
        const uint16_t mode = GP4_GLOBAL(uint16_t, 0x00866e8c);
        if (mode != 1u) {
            setFlags(r, sub16(mode, 1u));
            return;
        }
    }
    gp4::call_regs(0x00442baa, *r);
    setFlags(r, add16(counter, 1u));
    counter = static_cast<uint16_t>(counter + 1u);
    r->ecx &= 0xffff0000u;
    due = 0u;
    state = 0u;
}
GP4_IMPL(0x00442988, FUN_00442988, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:u32, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trackgeo_008943a0+tbl_00868598+state_0041104f+flag_007a97a0+cars_0066fef0+list_0066fe3c+state_00412af5+state_00403800+state_0040f629+lanes_0040da62+pend_0040daeb+ranges_004322a6+state_0040705f+frac_0040705f+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+pit_00406dbb+state_0040759e+trackfar_008943a0+segflag_004381ab+ctl_007ab548+carst_0040f6d0+carf_00410d04+counter_0040f0dd+counts_0066ec49+state_004059ef+state_004094e3+div_00403bb6+trk_007ad894+circ_007adb50+lvl_007adb86+codes_0043ba12+curves_0062ecb0+wheel_00410a1c+state_0066d8c0+words_0062d210+state_0042595c+carno_0066ff6c+state_0040f112+gain_00408790+state_004087aa+div_0060138c+state_0040860a+state_00405f77+levels_006f2cc4+tbl_0066d94c+state_0043c1ba+state_0043c31b+state_0043ca8b+state_0043c4d9+tabs_0043ced2+state_00411109+scale_0066ec60+keys_00408c3e+state_00431ffb+start_004098fd+car_00442baa+state_0040fa01+lap_0043b482+clock_0066d814+list_00443278+order_00675958+stream_0062d1a0+state_00443996; cases=600")
