#include <gp4/gp4.h>

// imul then shrd eax, edx, n: the low dword of the signed 64-bit product shifted right by n; the high
// dword of the product (edx) goes to `high`.
static inline uint32_t mulShr(uint32_t a, uint32_t b, int n, uint32_t* high) {
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(a)) * static_cast<int64_t>(static_cast<int32_t>(b));
    *high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> n);
}

// pushal ... popal routine. Latches the five generator bytes 0x0066d8bf..c3 into 0x0062d1b3..b7, then steps
// each of the 0x140 0xe0-byte records at 0x00854414 whose dword +0x4c has bit 0 set:
//  - flag 0x2000 of +0x98: 0x00424e0e tests slot byte +0xdc; a clear slot frees it (0x0051fc58) and
//    deactivates the record. Otherwise the countdown +0x50 drops by [0x603854]; below zero deactivates it.
//  - flag 0x10: 0x00443e69 re-attaches the record to its parent; CF deactivates it.
//  - flag 0x2000 (live slot): 0x00424371 reloads it from its slot entry.
//  - otherwise the velocities +0x24/+0x28/+0x2c are damped by +0x70/+0x6c (+0x34 is the vertical
//    acceleration) and integrated into +0x18/+0x1c/+0x20, scaled by [0x603854] >> 16; 0x00443dd1 runs
//    between the vertical update and the floor test (+0x88 + +0xbc, unless flag 0x1000): below the floor
//    the downward velocity bounces (scaled by +0xb8) and the height is reflected. Then 0x0040e735 when flag
//    0x200 is set, and 0x00444008.
// The callees see the registers the original leaves; popal restores them all and the flags are those of the
// final `dec ecx` (ZF/PF set, CF clear from the last `add esi, 0xe0`).
// FUNCTION: GP4 0x004440be
void __cdecl FUN_004440be(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x0062d1b3) = GP4_GLOBAL(uint8_t, 0x0066d8bf);
    GP4_GLOBAL(uint8_t, 0x0062d1b4) = GP4_GLOBAL(uint8_t, 0x0066d8c0);
    GP4_GLOBAL(uint8_t, 0x0062d1b5) = GP4_GLOBAL(uint8_t, 0x0066d8c1);
    GP4_GLOBAL(uint8_t, 0x0062d1b6) = GP4_GLOBAL(uint8_t, 0x0066d8c2);
    GP4_GLOBAL(uint8_t, 0x0062d1b7) = GP4_GLOBAL(uint8_t, 0x0066d8c3);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    uint8_t* rec = GP4_ARRAY(uint8_t, 0x00854414);
    for (uint32_t count = 0x140u; count != 0u; --count, rec += 0xe0) {
        c.ecx = count;
        c.esi = reinterpret_cast<uint32_t>(rec);
        if ((GP4_FIELD(uint32_t, rec, 0x4c) & 1u) == 0u) {
            continue;
        }
        if ((GP4_FIELD(uint32_t, rec, 0x98) & 0x2000u) != 0u) {
            c.eax = GP4_FIELD(uint8_t, rec, 0xdc);
            gp4::call_regs(0x00424e0e, c);
            if ((c.eflags & 1u) == 0u) {
                gp4::call_regs(0x0051fc58, c);
                GP4_FIELD(uint32_t, rec, 0x4c) &= ~1u;
                continue;
            }
        } else {
            const uint32_t rate = GP4_GLOBAL(uint32_t, 0x00603854);
            c.eax = rate;
            const uint32_t left = GP4_FIELD(uint32_t, rec, 0x50) - rate;
            GP4_FIELD(uint32_t, rec, 0x50) = left;
            if (static_cast<int32_t>(left) < 0) {
                GP4_FIELD(uint32_t, rec, 0x4c) &= ~1u;
                continue;
            }
        }

        if ((GP4_FIELD(uint32_t, rec, 0x98) & 0x10u) != 0u) {
            gp4::call_regs(0x00443e69, c);
            if ((c.eflags & 1u) != 0u) {
                GP4_FIELD(uint32_t, rec, 0x4c) &= ~1u;
                continue;
            }
        }
        if ((GP4_FIELD(uint32_t, rec, 0x98) & 0x2000u) != 0u) {
            c.eax = reinterpret_cast<uint32_t>(rec);
            gp4::call_regs(0x00424371, c);
            continue;
        }

        uint32_t high = 0u;
        uint32_t rate = GP4_GLOBAL(uint32_t, 0x00603854);
        uint32_t term = 0u - mulShr(GP4_FIELD(uint32_t, rec, 0x24), GP4_FIELD(uint32_t, rec, 0x70), 14, &high);
        GP4_FIELD(uint32_t, rec, 0x24) += mulShr(term, rate, 16, &high);
        GP4_FIELD(uint32_t, rec, 0x18) += mulShr(GP4_FIELD(uint32_t, rec, 0x24), rate, 16, &high);
        term = GP4_FIELD(uint32_t, rec, 0x34) - mulShr(GP4_FIELD(uint32_t, rec, 0x28), GP4_FIELD(uint32_t, rec, 0x6c), 14, &high);
        term = mulShr(term, rate, 16, &high);
        GP4_FIELD(uint32_t, rec, 0x28) += term;
        c.eax = term;
        c.edx = high;
        gp4::call_regs(0x00443dd1, c);

        const uint32_t floorHeight = GP4_FIELD(uint32_t, rec, 0x88) + GP4_FIELD(uint32_t, rec, 0xbc);
        c.ebp = floorHeight;
        rate = GP4_GLOBAL(uint32_t, 0x00603854);
        const uint32_t height = mulShr(GP4_FIELD(uint32_t, rec, 0x28), rate, 16, &high) + GP4_FIELD(uint32_t, rec, 0x1c);
        if ((GP4_FIELD(uint32_t, rec, 0x98) & 0x1000u) != 0u
            || static_cast<int32_t>(height) >= static_cast<int32_t>(floorHeight)) {
            GP4_FIELD(uint32_t, rec, 0x1c) = height;
        } else {
            const uint32_t vertical = GP4_FIELD(uint32_t, rec, 0x28);
            if (static_cast<int32_t>(vertical) < 0) {
                GP4_FIELD(uint32_t, rec, 0x28) = mulShr(0u - vertical, GP4_FIELD(uint32_t, rec, 0xb8), 14, &high);
            }
            const uint32_t floorAgain = GP4_FIELD(uint32_t, rec, 0x88) + GP4_FIELD(uint32_t, rec, 0xbc);
            const int32_t below = static_cast<int32_t>(floorAgain - GP4_FIELD(uint32_t, rec, 0x1c));
            GP4_FIELD(uint32_t, rec, 0x1c) = static_cast<uint32_t>(below >> 1) + floorAgain;
        }

        rate = GP4_GLOBAL(uint32_t, 0x00603854);
        term = 0u - mulShr(GP4_FIELD(uint32_t, rec, 0x2c), GP4_FIELD(uint32_t, rec, 0x70), 14, &high);
        GP4_FIELD(uint32_t, rec, 0x2c) += mulShr(term, rate, 16, &high);
        term = mulShr(GP4_FIELD(uint32_t, rec, 0x2c), rate, 16, &high);
        GP4_FIELD(uint32_t, rec, 0x20) += term;
        c.eax = term;
        c.edx = high;
        if ((GP4_FIELD(uint32_t, rec, 0x98) & 0x200u) != 0u) {
            gp4::call_regs(0x0040e735, c);
        }
        gp4::call_regs(0x00444008, c);
    }

    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x004440be, FUN_004440be, "regs() -> (cf, pf, af, zf, sf, of); wall=1800; globals=layout=trackgeo_008943a0+tbl_00868598+state_0040705f+state_00407af6+state_0040759e+trackfar_008943a0+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+pit_00406dbb+segflag_004381ab+w6_008943a0+rate_00603854+state_0066d8c0+parts_00854414")
