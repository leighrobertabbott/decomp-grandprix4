#include <gp4/gp4.h>
#include <gp4/flags.h>

// Multiplayer slot refresh (pushal/popal). When the host id check 0x00541a0a leaves ZF set (this
// machine's id [0x00b1bb40] equals the host id [0x00ae6b64]), walks the 22 slots of 0x1c bytes at
// 0x00b1b8d8 and rewrites the flag byte +0x14 of every used slot (first dword nonzero): 0 for the slot
// of the host id itself, otherwise 1 when 0x0053fe7a finds no key for the slot id and 0 when it does.
// popal restores every GPR and keeps the flags of the last writer: those of the call when the ids differ,
// else of the closing `or ecx, ecx` (ecx = 0).
// FUNCTION: GP4 0x0053fe36
void __cdecl FUN_0053fe36(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    gp4::call_regs(0x00541a0a, c);
    if ((c.eflags & 0x40u) == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, c.eflags);
        return;
    }

    uint8_t* slot = GP4_ARRAY(uint8_t, 0x00b1b8d8);
    for (uint32_t left = 0x16u; left != 0u; --left, slot += 0x1c) {
        const uint32_t id = GP4_FIELD(uint32_t, slot, 0);
        if (id == 0u) {
            continue;
        }
        const uint32_t host = GP4_GLOBAL(uint32_t, 0x00ae6b64);
        if (id == host) {
            GP4_FIELD(uint8_t, slot, 0x14) = 0u;
        } else {
            c.eax = id;
            c.ecx = left;
            c.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(slot));
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(id, host));
            gp4::call_regs(0x0053fe7a, c);
            GP4_FIELD(uint8_t, slot, 0x14) = (c.eax == 0u) ? 1u : 0u;
        }
    }

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(0u));
}
GP4_IMPL(0x0053fe36, FUN_0053fe36, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slots_00541d5a+keys_007c2a1c+host_00b1bb40")
