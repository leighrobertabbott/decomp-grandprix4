#include <gp4/gp4.h>

static inline void loadRegs(gp4::Regs& c, uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx,
                            uint32_t esi, uint32_t edi, uint32_t ebp) {
    c.eax = eax;
    c.ebx = ebx;
    c.ecx = ecx;
    c.edx = edx;
    c.esi = esi;
    c.edi = edi;
    c.ebp = ebp;
}

// Walks the cars queued in the table 0x6078e4 (indices [0x6078dc] .. [0x6078e0]) for the car in esi
// (bp = the reference value compared with word +0x68) and appends to the table 0x604294 (edi = its
// count) every car that is close enough (0x0042dd4b gap below [0x603ae8], not [0x604280], not already
// listed, 0x40dc8a / +0xe2 distance at least [0x603b28]). Cars with word +0x68 above bp are skipped by
// advancing the start index [0x6078dc]. Returns eax (last intermediate), ebx (last car), edx (= end
// index) and edi (the new count); the flags are those of the final `cmp edx,[0x6078e0]` (equal).
// FUNCTION: GP4 0x0042e06a
void __cdecl FUN_0042e06a(gp4::Regs* r) {
    const uint32_t esi = r->esi;
    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(esi));
    const uint32_t ebp = r->ebp;
    const int16_t bp = static_cast<int16_t>(ebp);
    const uint32_t ecx = r->ecx;

    uint32_t eax = r->eax;
    uint32_t ebx = r->ebx;
    uint32_t edi = r->edi;
    uint32_t edx = GP4_GLOBAL(uint32_t, 0x006078dc);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    while (edx != GP4_GLOBAL(uint32_t, 0x006078e0)) {
        ebx = GP4_ARRAY(uint32_t, 0x006078e4)[edx];
        uint8_t* const body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(ebx));

        if (bp < GP4_FIELD(int16_t, body, 0x68)) {
            GP4_GLOBAL(uint32_t, 0x006078dc) += 1u;
        } else {
            if ((body[0xcc] & 0x40u) != 0u) {
                uint16_t ax = GP4_FIELD(uint16_t, body, 0x90);
                if (static_cast<int16_t>(ax) < 0) ax = static_cast<uint16_t>(0u - ax);
                ax = static_cast<uint16_t>(ax - 0x280u);
                bool mark = static_cast<int16_t>(ax) < GP4_FIELD(int16_t, body, 0xb0);
                if (!mark) {
                    ax = static_cast<uint16_t>(GP4_FIELD(uint16_t, body, 0x90) - GP4_FIELD(uint16_t, body, 0x94));
                    if (static_cast<int16_t>(ax) < 0) ax = static_cast<uint16_t>(0u - ax);
                    mark = static_cast<int16_t>(ax) < 0x280;
                }
                eax = (eax & 0xffff0000u) | ax;
                if (mark) car[0xca] = static_cast<uint8_t>(car[0xca] | 0x40u);
            }

            loadRegs(c, eax, ebx, ecx, edx, esi, GP4_FIELD(uint32_t, car, 0x1c), ebp);
            gp4::call_regs(0x0042dd4b, c);
            eax = c.eax;

            if ((c.eflags & 1u) == 0u) {
                if (static_cast<int32_t>(eax) < 0) eax = 0u - eax;
                uint16_t ax = static_cast<uint16_t>(eax);
                if ((body[0x203] & 1u) != 0u && (car[0xfe] & 0x40u) == 0u && GP4_FIELD(uint16_t, car, 0xea) == 0u) {
                    ax = static_cast<uint16_t>(ax - 0x200u);
                    eax = (eax & 0xffff0000u) | ax;
                }
                if (static_cast<int16_t>(ax) < GP4_GLOBAL(int16_t, 0x00603ae8)) {
                    loadRegs(c, eax, ebx, ecx, edx, esi, edi, ebp);
                    gp4::call_regs(0x0042dee4, c);

                    if (ebx != 0u && ebx != GP4_GLOBAL(uint32_t, 0x00604280) && bp >= GP4_FIELD(int16_t, body, 0x68)) {
                        bool listed = false;
                        eax = edi - 1u;
                        while (static_cast<int32_t>(eax) >= 0) {
                            if (ebx == GP4_ARRAY(uint32_t, 0x00604294)[eax]) {
                                listed = true;
                                break;
                            }
                            eax = eax - 1u;
                        }
                        if (!listed) {
                            uint16_t gap;
                            if ((car[0x9c] & 2u) == 0u && (body[0x9c] & 2u) == 0u) {
                                gap = static_cast<uint16_t>(GP4_FIELD(uint16_t, body, 0xe2) - GP4_FIELD(uint16_t, car, 0xe2));
                                eax = (eax & 0xffff0000u) | gap;
                            } else {
                                loadRegs(c, eax, ebx, ecx, edx, esi, edi, ebp);
                                gp4::call_regs(0x0040dc8a, c);
                                eax = c.eax;
                                gap = static_cast<uint16_t>(eax);
                            }
                            if (static_cast<int16_t>(gap) >= GP4_GLOBAL(int16_t, 0x00603b28)) {
                                GP4_ARRAY(uint32_t, 0x00604294)[edi] = ebx;
                                edi = edi + 1u;
                            }
                        }
                    }
                }
            }
        }
        edx = edx + 1u;
    }

    r->eax = eax;
    r->ebx = ebx;
    r->edx = edx;
    r->edi = edi;
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0042e06a, FUN_0042e06a, "regs(eax:u32, ebx:u32, ecx:u32, esi:rec[0x66fef0,0x41c,0..21], edi:i32[0..6], ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=pair_0042e8e6+cars_0066fef0+carf_004036ca+carea_0042f6ee+flags_00408f7b+cars_0042dd4b+state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+win_0042b10d+ring_00405a12+links_0042b3c8+links_00405a12+step_0042b17e+segw_0042b3fd+walk_0042e06a; maxinsn=400000; cases=800")
