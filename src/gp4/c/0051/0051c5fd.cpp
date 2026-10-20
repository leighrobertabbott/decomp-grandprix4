#include <gp4/gp4.h>
#include <gp4/flags.h>

// Address of word `index` of the slot table at 0x009e1b20.
static __declspec(noinline) uint32_t SlotAddress(uint32_t index) {
    return 0x009e1b20u + index * 2u;
}

// `pushal ; ... ; popal`: walks the [0x0096a368] 0x20-byte records at [0x009658fc]. A record whose
// flag word (+0x1a, copied to 0x009679c2) has bits 4 and 0x20 set looks its id (word +0x12, copied
// to 0x009679a4) up in the slot table 0x009e1b20; unless the slot is 0xffff, the texture entry
// 0x006975ec + slot is remapped in place by 0x0051c59a. A zero count runs 2^32 times (do-while).
// popal restores every register; the flags are those of the final `cmp ebp, [0x96a368]` (equal).
// FUNCTION: GP4 0x0051c5fd
void __cdecl FUN_0051c5fd(gp4::Regs* r) {
    uint32_t record = GP4_GLOBAL(uint32_t, 0x009658fc);
    uint32_t index = 0u;
    uint32_t count;
    do {
        GP4_GLOBAL(uint16_t, 0x009679c2) = GP4_FIELD(uint16_t, record, 0x1a);
        if ((GP4_GLOBAL(uint16_t, 0x009679c2) & 4u) != 0u && (GP4_GLOBAL(uint16_t, 0x009679c2) & 0x20u) != 0u) {
            const uint32_t id = GP4_FIELD(uint16_t, record, 0x12);
            GP4_GLOBAL(uint16_t, 0x009679a4) = static_cast<uint16_t>(id);
            const uint16_t slot = GP4_GLOBAL(uint16_t, SlotAddress(id));
            if (slot != 0xffffu) {
                gp4::Regs c = *r;
                c.st_in = 0;
                c.st_out = 0;
                c.eax = slot + 0x006975ecu;
                gp4::call_regs(0x0051c59a, c);
            }
        }
        record += 0x20u;
        ++index;
        count = GP4_GLOBAL(uint32_t, 0x0096a368);
    } while (index != count);

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(index, count));
}
GP4_IMPL(0x0051c5fd, FUN_0051c5fd, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slots_0051cc50+lut_0051c59a+recs_0051c5fd")
