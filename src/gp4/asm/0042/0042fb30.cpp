#include <gp4/gp4.h>
#include <gp4/flags.h>

static inline uint8_t* carAt(uint32_t reg) {
    return reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(reg));
}

static inline void setLow16(uint32_t& reg, uint32_t value) {
    reg = (reg & 0xffff0000u) | (value & 0xffffu);
}

static inline bool carrySet(const gp4::Regs& c) {
    return (c.eflags & 1u) != 0u;
}

// movzx eax, word +0x1bc; cmp ax, -1; je; add eax, 0x66fef0; cmp ebx, eax; jne; mov word +0x1bc, 0xffff:
// forget the car the word names when it is the candidate in ebx.
static inline void dropLink(gp4::Regs& c, uint8_t* car) {
    c.eax = GP4_FIELD(uint16_t, car, 0x1bc);
    if (static_cast<uint16_t>(c.eax) != 0xffffu) {
        c.eax += 0x0066fef0u;
        if (c.ebx == c.eax) {
            GP4_FIELD(uint16_t, car, 0x1bc) = 0xffffu;
        }
    }
}

// esi = car. Clears the +0x134 bit 4 hold (and byte +0x235) once the time [0x66d814] reaches dword +0x1ec.
// With +0x134 bit 7 the car at +0x12c (and with bit 6 the one at +0x130) is dropped from the candidate table
// (0x0042fa30) and re-checked by 0x0042cd1e, whose target lateral [0x603c8c] is latched in 0x603c8e /
// 0x603c90; then 0x0042faf5 or the best queued candidate (0x0042fa52, key 0x6000 = none) is checked the
// same way, and depending on the outcome 0x0042c1f4 runs (or 0x0042d41d for key 0x6000). Finally, with
// +0x273 bit 6, +0x29c = [0x66d814] + ((rng byte << 6) * [0x603c80] >> 14) + [0x603c84] (or [0x603c88]
// when byte +0x12 of the car's segment +0x1c has bit 2). Registers are left as the callees and these steps
// leave them; the flags are those of the final add or of the +0x273 test.
// FUNCTION: GP4 0x0042fb30
void __cdecl FUN_0042fb30(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    uint8_t* car = carAt(c.esi);
    if ((car[0x134] & 0x10u) != 0u && GP4_FIELD(uint32_t, car, 0x1ec) != 0u) {
        const uint32_t elapsed = GP4_GLOBAL(uint32_t, 0x0066d814) - GP4_FIELD(uint32_t, car, 0x1ec);
        if (static_cast<int32_t>(elapsed) >= 0) {
            car[0x134] = static_cast<uint8_t>(car[0x134] & 0xefu);
            car[0x235] = 0u;
        }
    }

    bool pair = false;      // 0x0042fcbc: run 0x0042c1f4
    bool done = false;      // reached 0x0042fcc1 through the bit-7 path
    if ((car[0x134] & 0x80u) != 0u) {
        c.ebx = GP4_FIELD(uint32_t, car, 0x12c);
        gp4::call_regs(0x0042fa30, c);
        gp4::call_regs(0x0042cd1e, c);
        if (!carrySet(c)) {
            car = carAt(c.esi);
            setLow16(c.eax, GP4_GLOBAL(uint16_t, 0x00603c8c));
            GP4_GLOBAL(uint16_t, 0x00603c8e) = static_cast<uint16_t>(c.eax);
            done = true;
            bool viaQueue = true;
            if ((car[0x134] & 0x40u) != 0u) {
                c.ebx = GP4_FIELD(uint32_t, car, 0x130);
                gp4::call_regs(0x0042fa30, c);
                gp4::call_regs(0x0042cd1e, c);
                if (!carrySet(c)) {
                    setLow16(c.eax, GP4_GLOBAL(uint16_t, 0x00603c8c));
                    GP4_GLOBAL(uint16_t, 0x00603c90) = static_cast<uint16_t>(c.eax);
                    gp4::call_regs(0x0042faf5, c);
                    viaQueue = false;
                }
            }
            if (viaQueue) {
                // 0x0042fc4f
                gp4::call_regs(0x0042fa52, c);
                bool decided = false;
                if (!carrySet(c) && c.ebp != 0x6000u) {
                    GP4_GLOBAL(uint32_t, 0x00603c7c) = c.ebp;
                    gp4::call_regs(0x0042cd1e, c);
                    if (!carrySet(c)) {
                        setLow16(c.edx, GP4_GLOBAL(uint16_t, 0x00603c8c));
                        if (static_cast<int16_t>(c.edx) > GP4_GLOBAL(int16_t, 0x00603c8e)) {
                            pair = true;
                            decided = true;
                        } else {
                            dropLink(c, carAt(c.esi));
                        }
                    }
                }
                if (!decided) {
                    // 0x0042fc9a
                    c.ebx = GP4_FIELD(uint32_t, carAt(c.esi), 0x12c);
                    pair = c.ebx != 0u;
                }
            }
        }
    }

    if (!done) {
        car = carAt(c.esi);
        bool queueOnly = true;
        // 0x0042fbc5
        if ((car[0x134] & 0x40u) != 0u) {
            c.ebx = GP4_FIELD(uint32_t, car, 0x130);
            gp4::call_regs(0x0042fa30, c);
            gp4::call_regs(0x0042cd1e, c);
            if (!carrySet(c)) {
                queueOnly = false;
                setLow16(c.eax, GP4_GLOBAL(uint16_t, 0x00603c8c));
                GP4_GLOBAL(uint16_t, 0x00603c90) = static_cast<uint16_t>(c.eax);
                gp4::call_regs(0x0042fa52, c);
                bool decided = false;
                if (!carrySet(c) && c.ebp != 0x6000u) {
                    GP4_GLOBAL(uint32_t, 0x00603c7c) = c.ebp;
                    gp4::call_regs(0x0042cd1e, c);
                    if (!carrySet(c)) {
                        setLow16(c.edx, GP4_GLOBAL(uint16_t, 0x00603c8c));
                        if (static_cast<int16_t>(c.edx) < GP4_GLOBAL(int16_t, 0x00603c90)) {
                            pair = true;
                            decided = true;
                        } else {
                            dropLink(c, carAt(c.esi));
                        }
                    }
                }
                if (!decided) {
                    // 0x0042fc43
                    c.ebx = GP4_FIELD(uint32_t, carAt(c.esi), 0x130);
                    pair = c.ebx != 0u;
                }
            }
        }
        if (queueOnly) {
            // 0x0042fca6
            gp4::call_regs(0x0042fa52, c);
            if (!carrySet(c)) {
                if (c.ebp != 0x6000u) {
                    pair = true;
                } else {
                    gp4::call_regs(0x0042d41d, c);
                }
            }
        }
    }

    if (pair) {
        gp4::call_regs(0x0042c1f4, c);
    }

    // 0x0042fcc1
    car = carAt(c.esi);
    uint32_t flags = gp4::flags::logic8(car[0x273] & 0x40u);
    if ((car[0x273] & 0x40u) != 0u) {
        gp4::call_regs(0x00401462, c);
        const uint32_t pick = (c.eax & 0xffu) << 6;
        const int64_t product = static_cast<int64_t>(static_cast<int32_t>(pick))
            * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603c80));
        c.edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
        c.eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14) + GP4_GLOBAL(uint32_t, 0x00603c84);
        car = carAt(c.esi);
        c.edi = GP4_FIELD(uint32_t, car, 0x1c);
        if ((carAt(c.edi)[0x12] & 4u) != 0u) {
            c.eax = GP4_GLOBAL(uint32_t, 0x00603c88);
        }
        const uint32_t now = GP4_GLOBAL(uint32_t, 0x0066d814);
        GP4_FIELD(uint32_t, car, 0x29c) = now + c.eax;
        flags = gp4::flags::add32(now, c.eax);
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0042fb30, FUN_0042fb30, "regs(eax:u32, ecx:u32, edx:u32, edi:rec[0x8943a0,0x160,0..25], ebp:u32, ebx:u32, esi:rec[0x66fef0,0x41c,0..21]) -> (eax, ebx, ecx, edx, edi, ebp, cf, pf, af, zf, sf, of); maxinsn=2000000; globals=fuzz:i32+layout=list_0066fe3c+pair_0042e8e6+cars_0066fef0+carf_004036ca+carea_0042f6ee+flags_00408f7b+cars_0042dd4b+state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+win_0042b10d+ring_00405a12+links_0042b3c8+links_00405a12+step_0042b17e+segw_0042b3fd+rel_0042c0c4+state_0042b740+state_0042bb90+bounds_0042e330+link_0042a120+state_0042b8bb+state_004059ef+eq_0042ba87+state_0066d8c0+cand_0042faf5+ring_0042c1f4+cars_0042fb30")
