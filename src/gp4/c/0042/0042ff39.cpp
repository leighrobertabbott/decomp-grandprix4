#include <gp4/gp4.h>
#include <gp4/flags.h>

static inline uint8_t* ptrOf(uint32_t address) {
    return reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(address));
}

static inline uint8_t& byteAt(uint32_t base, uint32_t offset) {
    return GP4_FIELD(uint8_t, ptrOf(base), offset);
}

static inline int16_t& wordAt(uint32_t base, uint32_t offset) {
    return GP4_FIELD(int16_t, ptrOf(base), offset);
}

static inline uint32_t& dwordAt(uint32_t base, uint32_t offset) {
    return GP4_FIELD(uint32_t, ptrOf(base), offset);
}

static inline void setLow16(uint32_t& reg, int32_t value) {
    reg = (reg & 0xffff0000u) | (static_cast<uint32_t>(value) & 0xffffu);
}

static inline bool carry(const gp4::Regs* r) {
    return (r->eflags & gp4::flags::CF) != 0u;
}

// clc / stc: keep the flags of the last flag writer, replace CF.
static inline void finish(gp4::Regs* r, uint32_t cf) {
    r->eflags = (r->eflags & ~gp4::flags::CF) | cf;
}

// FUNCTION: GP4 0x0042ff39
// Overtaking decision for the car esi against the car ebx that 0x0042e783 (else 0x0042e8e6) picks
// (edi = the car's track segment). Stores the pair gap word in [0x603e70], the speed-dependent
// values +0xe0/+0xc8/+0x102, links the cars (+0x122 bit 1, +0x124, due time +0xf0) when ebx is the
// car [0x6043e0] just ahead, then runs the overtaking chain 0x42da35 / 0x42ac9c and the lane change
// 0x42a354 (stc when it succeeds) or the follow-up 0x42ecbf / 0x42f05a (clc).
void __cdecl FUN_0042ff39(gp4::Regs* r) {
    gp4::call_regs(0x0042e783, *r);
    if (carry(r)) {
        gp4::call_regs(0x0042e8e6, *r);
        if (carry(r)) {
            finish(r, 0u);
            return;
        }
    }

    // 0x42ff4b
    dwordAt(r->esi, 0x110) = r->ebx;
    GP4_GLOBAL(uint8_t, 0x00604156) = 0u;
    GP4_GLOBAL(uint8_t, 0x00604157) = 0xffu;
    GP4_GLOBAL(uint8_t, 0x00604151) = 0u;
    GP4_GLOBAL(uint8_t, 0x00604152) = 0u;
    if ((byteAt(r->esi, 0x9c) & 2u) != 0u || (byteAt(r->ebx, 0x9c) & 2u) != 0u) {
        gp4::call_regs(0x0040dc8a, *r);
    } else {
        setLow16(r->eax, wordAt(r->ebx, 0xe2) - wordAt(r->esi, 0xe2));
    }
    const int16_t gap = static_cast<int16_t>(r->eax);
    GP4_GLOBAL(int16_t, 0x00603e70) = gap;
    {
        const int16_t offset = (byteAt(r->esi, 0xce) & 0x80u) != 0u
            ? GP4_GLOBAL(int16_t, 0x0062d208) : GP4_GLOBAL(int16_t, 0x00603ba2);
        const int16_t window = static_cast<int16_t>(gap - offset);
        setLow16(r->ecx, window);
        const int16_t speed = static_cast<int16_t>(wordAt(r->esi, 0x9a) >> 7);
        setLow16(r->edx, speed);
        if (!(speed < window)) {
            byteAt(r->esi, 0xcb) |= 1u;
        }
    }
    if ((byteAt(r->esi, 0xe5) & 4u) == 0u) {
        gp4::call_regs(0x0042ac43, *r);
    } else {
        int16_t lateral = static_cast<int16_t>(wordAt(r->esi, 0x90) - wordAt(r->ebx, 0x90));
        if (lateral < 0) {
            lateral = static_cast<int16_t>(-lateral);
        }
        setLow16(r->edx, lateral);
        if (lateral < GP4_GLOBAL(int16_t, 0x00603aec)) {
            int16_t room = static_cast<int16_t>(GP4_GLOBAL(int16_t, 0x00603af6) - static_cast<int16_t>(r->eax));
            setLow16(r->ecx, room);
            if (room >= 0) {
                if (room > GP4_GLOBAL(int16_t, 0x00603af8)) {
                    room = GP4_GLOBAL(int16_t, 0x00603af8);
                    setLow16(r->ecx, room);
                }
                const int64_t first = static_cast<int64_t>(static_cast<int32_t>(room))
                    * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00601c68));
                const int32_t scaled = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(first) >> 14));
                const int64_t second = static_cast<int64_t>(scaled)
                    * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603854));
                r->edx = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 32);
                wordAt(r->esi, 0xe0) = static_cast<int16_t>(static_cast<uint32_t>(static_cast<uint64_t>(second) >> 16));
            }
        }
    }

    // 0x430035
    gp4::call_regs(0x0042a6dc, *r);
    wordAt(r->esi, 0xc8) = static_cast<int16_t>(r->ecx);
    gp4::call_regs(0x0042a533, *r);
    wordAt(r->esi, 0x102) = static_cast<int16_t>(r->ecx);

    enum { kLinkCheck, kChainA, kChainB, kChainC, kTail, kFollow } next = kLinkCheck;
    if ((byteAt(r->esi, 0x179) & 0x80u) == 0u) {
        if ((byteAt(r->ebx, 0x122) & 8u) != 0u && r->esi == dwordAt(r->ebx, 0x124)
            && wordAt(r->esi, 0xea) != 0x18 && (static_cast<uint16_t>(wordAt(r->edi, 0x1a)) & 0x1000u) == 0u
            && (byteAt(r->ebx, 0xe5) & 4u) != 0u) {
            next = kChainC;    // 0x4302d7
        } else if ((byteAt(r->ebx, 0xcc) & 0x10u) != 0u) {
            // 0x43008f
            const int16_t ahead = GP4_GLOBAL(int16_t, 0x00603e70);
            setLow16(r->eax, ahead);
            bool steer = true;
            if (!(ahead < GP4_GLOBAL(int16_t, 0x00603b54))) {
                const int16_t closing = static_cast<int16_t>(wordAt(r->esi, 0x9a) - wordAt(r->ebx, 0x9a));
                const int16_t margin = static_cast<int16_t>(ahead - GP4_GLOBAL(int16_t, 0x00603ba0));
                setLow16(r->ecx, margin);
                const int16_t shifted = static_cast<int16_t>(closing >> 4);
                setLow16(r->edx, shifted);
                steer = !(shifted < margin);
            }
            if (steer) {
                gp4::call_regs(0x0042ac9c, *r);
                next = kChainA;    // 0x430298
            }
        }
    }

    if (next == kLinkCheck) {
        // 0x4300c9
        if (r->ebx == GP4_GLOBAL(uint32_t, 0x006043e0) && (byteAt(r->ebx, 0x122) & 2u) == 0u) {
            bool candidate;
            if ((byteAt(r->ebx, 0x203) & 0x10u) != 0u) {
                candidate = true;
            } else if (static_cast<int8_t>(GP4_GLOBAL(uint8_t, 0x007c2ea9)) < 0) {
                const uint8_t order = static_cast<uint8_t>(byteAt(r->esi, 0x70) - byteAt(r->ebx, 0x70));
                r->edx = (r->edx & 0xffffff00u) | order;
                candidate = order != 0u && static_cast<int8_t>(order) >= 0;
            } else {
                candidate = false;
            }
            if (candidate && wordAt(r->ebx, 0xea) == 0) {
                bool open = true;
                if ((byteAt(r->ebx, 0x71) & 0x80u) != 0u) {
                    const uint32_t segment = dwordAt(r->ebx, 0x1c);
                    if (segment < GP4_GLOBAL(uint32_t, 0x00891500)
                        && segment + 0x6e00u >= GP4_GLOBAL(uint32_t, 0x00866f00)) {
                        open = false;
                    }
                }
                if (open) {
                    const int16_t lead = static_cast<int16_t>(wordAt(r->ebx, 0x68) - wordAt(r->esi, 0x68));
                    setLow16(r->edx, lead);
                    if (lead >= 0 && lead < GP4_GLOBAL(int16_t, 0x00603b10)) {
                        byteAt(r->ebx, 0x122) |= 2u;
                        dwordAt(r->ebx, 0x124) = r->esi;
                        const uint32_t savedEax = r->eax;
                        gp4::call_regs(0x00401462, *r);
                        const uint32_t delay = (r->eax & 0xffu) << 6;
                        dwordAt(r->ebx, 0xf0) = GP4_GLOBAL(uint32_t, 0x0066d814);
                        dwordAt(r->ebx, 0xf0) += delay;
                        r->eax = savedEax;
                    }
                }
            }
        }

        // 0x430172
        next = kFollow;
        if ((byteAt(r->esi, 0x122) & 0x10u) == 0u) {
            bool blocked = false;
            if ((byteAt(r->ebx, 0x203) & 1u) == 0u && (byteAt(r->edi, 0x51) & 0x20u) != 0u) {
                gp4::call_regs(0x0042e968, *r);
                blocked = !carry(r);
            }
            if (!blocked && (byteAt(r->esi, 0xcb) & 4u) == 0u && wordAt(r->esi, 0xea) != 0x18
                && (static_cast<uint16_t>(wordAt(r->edi, 0x1a)) & 0x1000u) == 0u
                && !((byteAt(r->esi, 0x71) & 0x80u) != 0u && (byteAt(r->edi, 0x64) & 0x80u) != 0u)) {
                gp4::call_regs(0x0042da35, *r);
                if (GP4_GLOBAL(uint8_t, 0x00604156) == 0xffu) {
                    const int16_t ahead = GP4_GLOBAL(int16_t, 0x00603e70);
                    setLow16(r->eax, ahead);
                    const int16_t closing = static_cast<int16_t>(wordAt(r->esi, 0x9a) - wordAt(r->ebx, 0x9a));
                    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(closing))
                        * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603bcc));
                    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
                    const int16_t margin = static_cast<int16_t>(ahead - GP4_GLOBAL(int16_t, 0x00603ba0));
                    setLow16(r->ecx, margin);
                    bool go = true;
                    if (!(static_cast<int16_t>(r->edx) >= margin)) {
                        const int16_t pace = wordAt(r->esi, 0x102);
                        setLow16(r->edx, pace);
                        go = !(pace > GP4_GLOBAL(int16_t, 0x006039a2));
                    }
                    if (go) {
                        const int16_t shifted = static_cast<int16_t>(
                            static_cast<int16_t>(wordAt(r->esi, 0x9a) - wordAt(r->ebx, 0x9a)) >> 5);
                        setLow16(r->edx, shifted);
                        if (shifted < margin) {
                            next = kTail;
                        }
                    }
                }
            }
        }
    }

    if (next == kChainA) {
        // 0x430298
        gp4::call_regs(0x0042da35, *r);
        if (GP4_GLOBAL(uint8_t, 0x00604156) == 0xffu) {
            next = kTail;
        } else {
            r->edi = dwordAt(r->esi, 0x1c);
            bool overtake = true;
            if ((byteAt(r->edi, 0x12) & 4u) != 0u) {
                const int64_t product = static_cast<int64_t>(static_cast<int32_t>(static_cast<uint16_t>(wordAt(r->esi, 0xd8))))
                    * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603ddc));
                r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
                r->eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
                overtake = !(static_cast<int16_t>(r->eax) < wordAt(r->esi, 0x9a));
            }
            if (overtake) {
                gp4::call_regs(0x0042ac9c, *r);
                GP4_GLOBAL(uint8_t, 0x00604156) = 0xffu;
                next = kTail;
            } else {
                next = kFollow;
            }
        }
    } else if (next == kChainC) {
        // 0x4302d7
        gp4::call_regs(0x0042da35, *r);
        next = GP4_GLOBAL(uint8_t, 0x00604156) == 0xffu ? kTail : kFollow;
    }

    if (next == kTail) {
        // 0x4302e7
        int16_t lane = wordAt(r->esi, 0xea);
        if (lane < 0) {
            lane = static_cast<int16_t>(-lane);
        }
        setLow16(r->edx, lane);
        if (lane == 0x10) {
            if ((byteAt(r->ebx, 0xcc) & 0x10u) != 0u) {
                gp4::call_regs(0x0042e322, *r);
                dwordAt(r->esi, 0x118) = 0u;
                r->edx = 0x5000u;
                gp4::call_regs(0x0042a0fb, *r);
            }
        } else {
            gp4::call_regs(0x0042a354, *r);
            if (!carry(r)) {
                finish(r, gp4::flags::CF);
                return;
            }
        }
    }

    // 0x430329
    gp4::call_regs(0x0042ecbf, *r);
    if (!carry(r)) {
        gp4::call_regs(0x0042f05a, *r);
    }
    finish(r, 0u);
}
GP4_IMPL(0x0042ff39, FUN_0042ff39, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, ebp:u32, esi:rec[0x66fef0,0x41c,0..21], edi:rec[0x8943a0,0x160,0..25]) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=pair_0042e8e6+cars_0066fef0+carea_0042f6ee+flags_00408f7b+cars_0042dd4b+state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+win_0042b10d+ring_00405a12+links_0042b3c8+links_00405a12+step_0042b17e+segw_0042b3fd+bounds_0042e330+clock_0066d814+chain_0042dc79+list_0042dfd2+walk_0042e06a+walk_0042e191+link_0042e5bf+carf_00410d04+lap_0043b482+state_00426091+gap_0042a533+state_0042da35+state_0042fa52+state_0066d8c0+state_0042eee3+state_0042ecbf+state_0042efee+state_004059ef+state_0042f3c7+list_0066fe3c+cursor_006043fc+order_0042e97e; maxinsn=4000000; cases=2500; wall=1800")
