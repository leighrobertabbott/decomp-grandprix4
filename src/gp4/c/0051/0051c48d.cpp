#include <gp4/gp4.h>
#include <gp4/flags.h>

// Called by 0x0051c058 after a texture block of [0x96a374] 0x100-byte rows was copied to the
// running buffer pointer (old value in [0x965900], records at [0x965904], count [0x96a368]).
// Only when the mode byte [0x9e1b1c] is 10: q = 0x0051c45c() (q + q/2 == rows); the eight level
// pointers of every 0x20-byte record are written to a table at [0x965900] + (q + q/4)*0x100 + 0x80
// (an extra 0x80 gap after every fourth record): level k = base + (y >> k)*0x100 + (x >> k)
// + [0x9e1628 + 4k] (+ q*0x100 for k > 0), where x/y are the low byte / rest of the record's
// offset (+0 minus base); record +0 then points at its level table. Words +4/+6 are halved per
// level into [0x9679e0]/[0x9679e4] (kept >= 1). The end of the table raises the high-water mark
// [0x6975e0]. pushal/popal: registers are restored; flags are those of `cmp byte [0x9e1b1c], 10`
// when skipped, else of the final `cmp edi, [0x6975e0]`.
// FUNCTION: GP4 0x0051c48d
void __cdecl FUN_0051c48d(gp4::Regs* r) {
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x009e1b1c);
    if (mode != 0x0au) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(mode, 0x0au));
        return;
    }
    gp4::Regs c = *r;
    gp4::call_regs(0x0051c45c, c);
    const uint32_t q = c.eax;
    uint32_t out = ((q + (q >> 2)) << 8) + 0x80u + GP4_GLOBAL(uint32_t, 0x00965900);
    uint32_t rec = GP4_GLOBAL(uint32_t, 0x00965904);
    uint32_t index = 0;
    do {
        GP4_GLOBAL(uint32_t, 0x009679e0) = GP4_FIELD(uint16_t, rec, 4);
        GP4_GLOBAL(uint32_t, 0x009679e4) = GP4_FIELD(uint16_t, rec, 6);
        const uint32_t offset = GP4_FIELD(uint32_t, rec, 0) - GP4_GLOBAL(uint32_t, 0x00965900);
        GP4_GLOBAL(uint32_t, 0x009679e8) = offset & 0xffu;
        GP4_GLOBAL(uint32_t, 0x009679ec) = offset >> 8;
        GP4_FIELD(uint32_t, rec, 0) = out;
        for (int level = 0; level != 8; ++level) {
            uint32_t value = (GP4_GLOBAL(uint32_t, 0x009679ec) << 8) + GP4_GLOBAL(uint32_t, 0x009679e8)
                + GP4_ARRAY(uint32_t, 0x009e1628)[level];
            if (level != 0) {
                value += q << 8;
            }
            value += GP4_GLOBAL(uint32_t, 0x00965900);
            *reinterpret_cast<uint32_t*>(out) = value;
            out += 4u;
            GP4_GLOBAL(uint32_t, 0x009679ec) >>= 1;
            GP4_GLOBAL(uint32_t, 0x009679e8) >>= 1;
            GP4_GLOBAL(uint32_t, 0x009679e0) >>= 1;
            if (static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x009679e0)) < 1) {
                GP4_GLOBAL(uint32_t, 0x009679e0) = 1u;
            }
            GP4_GLOBAL(uint32_t, 0x009679e4) >>= 1;
            if (static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x009679e4)) < 1) {
                GP4_GLOBAL(uint32_t, 0x009679e4) = 1u;
            }
        }
        if ((index & 3u) == 3u) {
            out += 0x80u;
        }
        rec += 0x20u;
        ++index;
    } while (index != GP4_GLOBAL(uint32_t, 0x0096a368));
    const uint32_t mark = GP4_GLOBAL(uint32_t, 0x006975e0);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(out, mark));
    if (out >= mark) {
        GP4_GLOBAL(uint32_t, 0x006975e0) = out;
    }
}
GP4_IMPL(0x0051c48d, FUN_0051c48d, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=tex_0051c48d")
