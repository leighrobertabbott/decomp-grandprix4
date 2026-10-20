#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu-state step for the record in esi (GP3 heritage state machine at 0x0066d8d8..e6). When the "state
// changed" byte [0x0066d8e6] is not 0xff, the current state index [0x0066d8d8] moves through the second link
// table 0x00893fa0 (the new index is stored to [0x0066d8d8] and [0x008660a0], the timers [0x0066d8dc],
// [0x0066d8e0] and [0x008660a4] are cleared), [0x0066d8e6] becomes 0xff and bit 7 of record byte +0x88 is
// set. Then with the mode byte [0x0066d8e4] nonzero and record flag 0x10 (+0x6f) set, the link-table step
// 0x00521d6f runs; otherwise, with the mode 0xff and flag 4 or 8 set, the gain routine 0x00431da8 runs with
// eax = -1 (flag 4) or 1 (flag 8), bit 5 of +0x89 is set and [0x008914e4] becomes 0xff. eax holds the
// table entry or the +-1 direction; the flags are those of the last compare / test / or.
// FUNCTION: GP4 0x00521eda
void __cdecl FUN_00521eda(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t eax = r->eax;
    uint32_t flags = gp4::flags::sub8(GP4_GLOBAL(uint8_t, 0x0066d8e6), 0xffu);

    if (GP4_GLOBAL(uint8_t, 0x0066d8e6) != 0xffu) {
        eax = GP4_ARRAY(uint32_t, 0x00893fa0)[GP4_GLOBAL(uint32_t, 0x0066d8d8)];
        GP4_GLOBAL(uint32_t, 0x0066d8d8) = eax;
        GP4_GLOBAL(uint32_t, 0x008660a0) = eax;
        GP4_GLOBAL(uint32_t, 0x0066d8dc) = 0;
        GP4_GLOBAL(uint32_t, 0x0066d8e0) = 0;
        GP4_GLOBAL(uint32_t, 0x008660a4) = 0;
        GP4_GLOBAL(uint8_t, 0x0066d8e6) = 0xffu;
        rec[0x88] = static_cast<uint8_t>(rec[0x88] | 0x80u);
        flags = gp4::flags::logic8(rec[0x88]);
    }

    r->eax = eax;
    const uint32_t mode = GP4_GLOBAL(uint8_t, 0x0066d8e4);
    flags = gp4::flags::sub8(mode, 0u);
    if (mode != 0u) {
        flags = gp4::flags::logic8(rec[0x6f] & 0x10u);
        if ((rec[0x6f] & 0x10u) != 0u) {
            gp4::call_regs(0x00521d6f, *r);
            r->eflags = gp4::flags::merge(r->eflags, flags);
            return;
        }
    }

    flags = gp4::flags::sub8(GP4_GLOBAL(uint8_t, 0x0066d8e4), 0xffu);
    if (GP4_GLOBAL(uint8_t, 0x0066d8e4) == 0xffu) {
        r->eax = 0xffffffffu;
        flags = gp4::flags::logic8(rec[0x6f] & 4u);
        if ((rec[0x6f] & 4u) == 0u) {
            flags = gp4::flags::logic8(rec[0x6f] & 8u);
            if ((rec[0x6f] & 8u) == 0u) {
                r->eflags = gp4::flags::merge(r->eflags, flags);
                return;
            }
            r->eax = 1u;
        }
        gp4::call_regs(0x00431da8, *r);
        rec[0x89] = static_cast<uint8_t>(rec[0x89] | 0x20u);
        GP4_GLOBAL(uint8_t, 0x008914e4) = 0xffu;
        flags = gp4::flags::logic8(rec[0x89]);
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00521eda, FUN_00521eda, "regs(eax:u32, esi:ptr[0x1a4]:layout=rec_00431da8) -> (eax, cf, pf, af, zf, sf, of); globals=layout=links_00521d6f+state_004087aa+gain_00408790+sel_00522192")
