#include <gp4/gp4.h>
#include <gp4/flags.h>

static inline uint8_t* ptrOf(uint32_t address) {
    return reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(address));
}

static inline uint32_t addressOf(const void* p) {
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p));
}

// Car record at 0x66fef0 + the signed byte offset stored in an order-list word.
static inline uint32_t carAt(const int16_t* slot) {
    return addressOf(GP4_ARRAY(uint8_t, 0x0066fef0) + *slot);
}

// FUNCTION: GP4 0x00430ddd
// Per-frame pass over the car order list 0x66fe3c .. [0x6043f8]: resets the pair cache and the
// per-frame counters (0x431456, 0x430d07), then for every listed car esi (previous car in
// [0x6043e0], next one - wrapping to the list head - in [0x6043e4], cursor [0x6043fc]) clears its
// per-frame fields, runs the driving chain 0x42f450 / 0x430c8e (0x42aa47 / 0x42f8bf on clc) for
// cars without +0x7a bit 7, updates the +0x83 bit 4 / +0xcb bit 1 flags from the segment and
// runs 0x431702 or 0x431881. The largest gap [ebx+0x68] - [esi+0x68] between consecutive cars is
// kept in [0x603afa] (0x43b51c, car in [0x6043f0]); 0x43b576 runs when one was found.
void __cdecl FUN_00430ddd(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x00603e84) = 0xffffffffu;
    GP4_GLOBAL(uint32_t, 0x00603e88) = 0xffffffffu;
    GP4_GLOBAL(uint32_t, 0x00603e8c) = 0x4000u;
    gp4::call_regs(0x00431456, *r);
    GP4_GLOBAL(uint32_t, 0x006043f0) = 0u;
    GP4_GLOBAL(uint8_t, 0x00604154) = 0u;
    gp4::call_regs(0x00430d07, *r);

    int16_t* const list = GP4_ARRAY(int16_t, 0x0066fe3c);
    r->esi = carAt(list);
    r->edi = addressOf(list + 1);
    GP4_GLOBAL(uint32_t, 0x006043fc) = r->edi;
    r->eax = carAt(list + 1);
    GP4_GLOBAL(uint32_t, 0x006043e4) = r->eax;
    r->edi = GP4_GLOBAL(uint32_t, 0x006043f8);
    r->ebx = carAt(reinterpret_cast<const int16_t*>(ptrOf(r->edi)) - 1);
    GP4_GLOBAL(uint32_t, 0x006043e0) = r->ebx;
    gp4::call_regs(0x0043b559, *r);

    for (;;) {
        // 0x430eb4
        uint8_t* car = ptrOf(r->esi);
        GP4_FIELD(uint8_t, car, 0x123) = 0u;
        GP4_FIELD(uint8_t, car, 0x1c5) = 0u;
        GP4_GLOBAL(uint32_t, 0x006077dc) = 0u;
        GP4_FIELD(uint16_t, car, 0xca) = 0u;
        GP4_FIELD(uint8_t, car, 0x273) = 0u;
        GP4_FIELD(uint16_t, car, 0x174) = 8u;
        if ((GP4_FIELD(uint8_t, car, 0x7a) & 0x80u) == 0u) {
            r->edi = GP4_FIELD(uint32_t, car, 0x1c);
            uint8_t* segment = ptrOf(r->edi);
            GP4_FIELD(uint16_t, car, 0xe0) = 0u;
            GP4_FIELD(uint32_t, car, 0x110) = 0u;
            const uint8_t segmentFlags = GP4_FIELD(uint8_t, segment, 0x51);
            if ((segmentFlags & 0x20u) == 0u) {
                GP4_FIELD(uint8_t, car, 0x137) = 0u;
            }
            if ((segmentFlags & 0x80u) == 0u) {
                GP4_FIELD(uint8_t, car, 0x179) &= 0xfbu;
                GP4_FIELD(uint8_t, car, 0x135) &= 0xfdu;
                GP4_FIELD(uint8_t, car, 0xcc) &= 0xdfu;
                if ((segmentFlags & 1u) == 0u) {
                    GP4_FIELD(uint8_t, car, 0x135) &= 0xf7u;
                }
            }
            gp4::call_regs(0x0042f450, *r);
            gp4::call_regs(0x00430c8e, *r);
            if ((r->eflags & gp4::flags::CF) == 0u) {
                gp4::call_regs(0x0042aa47, *r);
                gp4::call_regs(0x0042f8bf, *r);
            }
            car = ptrOf(r->esi);
        }

        // 0x430f4c: edi is the segment (or, for a skipped car, the list pointer left in edi)
        if ((GP4_FIELD(uint16_t, ptrOf(r->edi), 0x1a) & 0x1000u) != 0u) {
            GP4_FIELD(uint8_t, car, 0x83) |= 0x10u;
            if ((GP4_FIELD(uint8_t, car, 0xca) & 0x40u) != 0u) {
                const int32_t speed = static_cast<int32_t>(GP4_FIELD(uint16_t, car, 0xd8));
                const int64_t product = static_cast<int64_t>(speed)
                    * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00601f04));
                r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
                int32_t limit = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14));
                if (limit < GP4_GLOBAL(int32_t, 0x00601f08)) {
                    limit = GP4_GLOBAL(int32_t, 0x00601f08);
                }
                r->edi = GP4_FIELD(uint32_t, car, 0x1c);
                if ((GP4_FIELD(uint8_t, ptrOf(r->edi), 0x12) & 4u) == 0u) {
                    if (limit > GP4_GLOBAL(int32_t, 0x00601f0c)) {
                        limit = GP4_GLOBAL(int32_t, 0x00601f0c);
                    }
                } else {
                    if (limit > GP4_GLOBAL(int32_t, 0x00601f10)) {
                        limit = GP4_GLOBAL(int32_t, 0x00601f10);
                    }
                }
                r->eax = static_cast<uint32_t>(limit);
                if (static_cast<int16_t>(limit) < GP4_FIELD(int16_t, car, 0x9a)) {
                    GP4_FIELD(uint8_t, car, 0xcb) |= 2u;
                }
            }
        } else {
            GP4_FIELD(uint8_t, car, 0x83) &= 0xefu;
        }

        // 0x430fc0
        if ((GP4_FIELD(uint8_t, car, 0xcc) & 2u) != 0u) {
            gp4::call_regs(0x00431702, *r);
        } else if ((GP4_FIELD(uint8_t, car, 0xe5) & 0x80u) != 0u) {
            gp4::call_regs(0x00431881, *r);
        }

        r->edi = GP4_GLOBAL(uint32_t, 0x006043fc);
        if (r->edi == GP4_GLOBAL(uint32_t, 0x006043f8)) {
            break;
        }

        // 0x430e57: advance to the next listed car
        r->ebx = r->esi;
        GP4_GLOBAL(uint32_t, 0x006043e0) = r->ebx;
        const int16_t* slot = reinterpret_cast<const int16_t*>(ptrOf(r->edi));
        const uint16_t offset = static_cast<uint16_t>(*slot);
        r->eax = (r->eax & 0xffff0000u) | offset;
        r->edi = addressOf(slot + 1);
        GP4_GLOBAL(uint32_t, 0x006043fc) = r->edi;
        r->esi = addressOf(GP4_ARRAY(uint8_t, 0x0066fef0) + static_cast<int16_t>(offset));
        const int16_t* following = reinterpret_cast<const int16_t*>(ptrOf(r->edi));
        if (r->edi == GP4_GLOBAL(uint32_t, 0x006043f8)) {
            following = list;
        }
        r->edx = carAt(following);
        GP4_GLOBAL(uint32_t, 0x006043e4) = r->edx;
        const uint16_t gap = static_cast<uint16_t>(GP4_FIELD(uint16_t, ptrOf(r->ebx), 0x68)
            - GP4_FIELD(uint16_t, ptrOf(r->esi), 0x68));
        r->eax = (r->eax & 0xffff0000u) | gap;
        if (static_cast<int16_t>(gap) > GP4_GLOBAL(int16_t, 0x00603afa)) {
            GP4_GLOBAL(uint16_t, 0x00603afa) = gap;
            gp4::call_regs(0x0043b51c, *r);
            GP4_GLOBAL(uint32_t, 0x006043f0) = r->esi;
        }
    }

    // 0x430ff0
    if (GP4_GLOBAL(uint32_t, 0x006043f0) != 0u) {
        gp4::call_regs(0x0043b576, *r);
    } else {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(0u));
    }
}
GP4_IMPL(0x00430ddd, FUN_00430ddd, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, ebp:u32, esi:u32, edi:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=pair_0042e8e6+cars_0066fef0+carea_0042f6ee+flags_00408f7b+cars_0042dd4b+state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+win_0042b10d+ring_00405a12+links_0042b3c8+links_00405a12+step_0042b17e+segw_0042b3fd+bounds_0042e330+clock_0066d814+chain_0042dc79+list_0042dfd2+walk_0042e06a+walk_0042e191+state_00426091+gap_0042a533+state_0042da35+state_0042fa52+state_0066d8c0+state_0042ecbf+state_0042efee+state_004059ef+state_0042f3c7+cars_0042f47f+list_0066fe3c+cursor_006043fc+order_0042e97e+carx_0042fe2f+flags_00430449+state_0042f8bf+state_0042f450+flags_0042aa47+state_00431838; maxinsn=30000000; wall=1800")
