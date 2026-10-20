#include <gp4/gp4.h>
#include <gp4/flags.h>

// Row tables of 0x38 bytes (14 dwords) per walk index; esi selects the dword in the row.
static __declspec(noinline) int32_t RowX(uint32_t offset) { return *reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(0x00871cb0u + offset)); }
static __declspec(noinline) int32_t RowY(uint32_t offset) { return *reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(0x00872ab0u + offset)); }
static __declspec(noinline) int32_t RowYNext(uint32_t offset) { return *reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(0x00872ae8u + offset)); }
static __declspec(noinline) int32_t RowZ(uint32_t offset) { return *reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(0x008738b0u + offset)); }

// For each of the ten slots esi = 0..9 (skipped when the word 0x00852df0 is below 2 and the byte +0x67 of
// the current edi record has the bit 0x00875db4[esi] set): 0x00876f58 = -0x4000 for slots 0..4, else
// 0x4000; 0x00552047 resets the clip state; then the ring records from [0x009631fc] are walked for
// [0x009631f4] = 0..[0x009631f8] (signed, inclusive). A record is marked visible (0x008746b0[index] = 0xff)
// when the slot mask 0x00875d8c[esi] meets the flag dwords [edi-0x134] | [edi+0x2c] (and, for the inner
// indices, [edi+0x38]), the row entry 0x00872ab0 or the next row's entry is nonzero, and the z entry
// 0x008738b0 has bit 2 clear (the CF of its `sar 3`); the three row entries >> 3 go to 0x008754ac/b0/b4
// on that path and the (empty) routine 0x0055200c is called. Otherwise the byte is cleared. The record
// pointer advances by 0x160 with the lap/pit wrap of 0x00552126. 0x0055200e follows the walk and
// 0x005522f9 runs when it returns CF = 0. Registers are what the last step leaves; the flags are those of
// the final `cmp esi, 10`.
// FUNCTION: GP4 0x0055218f
void __cdecl FUN_0055218f(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    uint32_t slot = 0u;
    do {
        c.esi = slot;
        const int16_t level = GP4_GLOBAL(int16_t, 0x00852df0);
        uint32_t arith = gp4::flags::sub16(static_cast<uint16_t>(level), 2u);
        bool run = true;
        if (level < 2) {
            c.eax = GP4_ARRAY(uint8_t, 0x00875db4)[slot];
            const uint32_t bits = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(c.edi + 0x67u)) & c.eax;
            arith = gp4::flags::logic8(bits);
            run = (bits == 0u);
        }
        c.eflags = gp4::flags::merge(c.eflags, arith);

        if (run) {
            c.eax = (static_cast<int32_t>(slot) < 5) ? 0xffffc000u : 0x4000u;
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(slot, 5u));
            GP4_GLOBAL(uint32_t, 0x00876f58) = c.eax;
            gp4::call_regs(0x00552047, c);

            uint32_t record = GP4_GLOBAL(uint32_t, 0x009631fc);
            if (slot == 3u) {
                c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(record, 0x008ff7a0u));
            } else {
                c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(slot, 3u));
            }
            GP4_GLOBAL(uint32_t, 0x009631f4) = 0u;
            const uint32_t mask = GP4_ARRAY(uint32_t, 0x00875d8c)[slot];
            do {
                const uint32_t index = GP4_GLOBAL(uint32_t, 0x009631f4);
                c.eax = GP4_FIELD(uint32_t, record, -0x134) | GP4_FIELD(uint32_t, record, 0x2c);
                c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(mask & c.eax));
                bool visible = false;
                if ((mask & c.eax) != 0u) {
                    c.eax = index;
                    bool pass = true;
                    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(index, 0u));
                    if (index != 0u) {
                        const uint32_t last = GP4_GLOBAL(uint32_t, 0x009631f8);
                        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(index, last));
                        if (index != last) {
                            c.eax = GP4_FIELD(uint32_t, record, 0x38);
                            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(mask & c.eax));
                            pass = ((mask & c.eax) != 0u);
                        }
                    }
                    if (pass) {
                        c.edx = index * 0x38u;
                        const uint32_t offset = c.edx + slot * 4u;
                        const int32_t y = RowY(offset);
                        bool entry = (y != 0);
                        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(static_cast<uint32_t>(y)));
                        if (!entry) {
                            const int32_t yNext = RowYNext(offset);
                            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(static_cast<uint32_t>(yNext)));
                            entry = (yNext != 0);
                        }
                        if (entry) {
                            GP4_GLOBAL(int32_t, 0x008754ac) = RowX(offset) >> 3;
                            GP4_GLOBAL(int32_t, 0x008754b0) = RowY(offset) >> 3;
                            const int32_t z = RowZ(offset);
                            const int32_t zs = z >> 3;
                            GP4_GLOBAL(int32_t, 0x008754b4) = zs;
                            c.eax = static_cast<uint32_t>(zs);
                            // sar eax, 3: CF = bit 2, AF = 0, OF = 0.
                            uint32_t sar = gp4::flags::logic32(static_cast<uint32_t>(zs));
                            sar |= (static_cast<uint32_t>(z) >> 2) & 1u;
                            c.eflags = gp4::flags::merge(c.eflags, sar);
                            c.ebx = GP4_GLOBAL(uint32_t, 0x009631f4);
                            gp4::Regs e = c;
                            e.edi = GP4_GLOBAL(uint32_t, 0x009631e8);
                            gp4::call_regs(0x0055200c, e);
                            c.eflags = e.eflags;
                            visible = ((c.eflags & gp4::CF) == 0u);
                        }
                    }
                }
                c.edx = GP4_GLOBAL(uint32_t, 0x009631f4);
                GP4_ARRAY(uint8_t, 0x008746b0)[c.edx] = visible ? 0xffu : 0u;

                GP4_GLOBAL(uint32_t, 0x009631f4) = GP4_GLOBAL(uint32_t, 0x009631f4) + 1u;
                record += 0x160u;
                if (record >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
                    if (record < GP4_GLOBAL(uint32_t, 0x00891500)) {
                        record -= GP4_GLOBAL(uint32_t, 0x00866ec4);
                    } else if (record >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                        record = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
                    }
                }
                c.edi = record;
                c.eax = GP4_GLOBAL(uint32_t, 0x009631f4);
                c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.eax, GP4_GLOBAL(uint32_t, 0x009631f8)));
            } while (static_cast<int32_t>(c.eax) <= static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x009631f8)));

            gp4::call_regs(0x0055200e, c);
            if ((c.eflags & gp4::CF) == 0u) {
                gp4::call_regs(0x005522f9, c);
            }
        }
        slot = slot + 1u;
        c.esi = slot;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(slot, 10u));
    } while (slot != 10u);

    *r = c;
}
GP4_IMPL(0x0055218f, FUN_0055218f, "regs(edi:rec[0x8943a0,0x160,0..17]) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=geom_008943a0+span_00551227+mask_005520a0+ring_00555b28+state_00552126+rows_0055218f")
