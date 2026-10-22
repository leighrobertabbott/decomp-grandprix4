#include <gp4/gp4.h>
#include <gp4/flags.h>

// 22 slots of 0x1c bytes at 0x00b1b8d8; the first dword of a slot is its id.
struct Slot_00b1b8d8 {
    uint32_t Id00;
    uint8_t Unknown04[0x18];
};
static_assert(sizeof(Slot_00b1b8d8) == 0x1c, "slot size");

// Looks the id in eax up in the 22-slot table: eax = the address of the first slot whose id equals it
// (flags of that `cmp`, all equal), else 0. When no slot matches and this machine's id [0x00b1bb40] differs
// from the host id [0x00ae6b64] (0x00541a0a, flags of its `cmp`), an id equal to [0x00b1bb40] with a nonzero
// byte [0x0082fffd] also runs the dialog routine 0x00540054 (pushal/popal). ebx and ecx are saved and
// restored; the flags are those of the last compare/callee that ran.
// FUNCTION: GP4 0x00541d06
void __cdecl FUN_00541d06(gp4::Regs* r) {
    const uint32_t id = r->eax;
    const Slot_00b1b8d8* slot = GP4_ARRAY(Slot_00b1b8d8, 0x00b1b8d8);

    for (uint32_t remaining = 22; remaining != 0; --remaining, ++slot) {
        if (id == slot->Id00) {
            r->eax = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(slot));
            r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(id, slot->Id00));
            return;
        }
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00541a0a, c);
    if ((c.eflags & gp4::flags::ZF) == 0u) {
        const uint32_t self = GP4_GLOBAL(uint32_t, 0x00b1bb40);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(id, self));
        if (id == self) {
            const uint8_t flag = GP4_GLOBAL(uint8_t, 0x0082fffd);
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(flag, 0u));
            if (flag != 0u) {
                gp4::call_regs(0x00540054, c);
            }
        }
    }

    r->eax = 0u;
    r->eflags = c.eflags;
}
GP4_IMPL(0x00541d06, FUN_00541d06, "regs(eax:u32{=0,1,7,0x1f1f,0xffffffff,5,0x12345678}) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slots_00541d5a+kind_00541cdb+slotsel_00540884+host_00b1bb40")
