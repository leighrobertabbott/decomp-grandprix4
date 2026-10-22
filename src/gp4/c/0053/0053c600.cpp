#include <gp4/gp4.h>
#include <gp4/flags.h>

// Menu keyboard-state step (sibling of the mouse step 0x0053c500, both called by 0x0053c394). Runs only
// when the bytes 0x0082fd49 and 0x0082fd79 are both 0xff. Peeks the key queue (0x0052d7c8: al = key,
// ah = its class). Latch bit 0 of [0x0082fd6b] set: a class-6 key equal to the latched key word
// [0x007ab00e] is popped and releases the latch (event 5); otherwise any non-zero class key is popped,
// and when the repeat countdown [0x007ab003] is 0 it is rearmed to 2 and, unless the latched key is
// Esc (0x1b), the stored position 0x82fd7c/0x82fd80 is replayed (event 3). Latch clear: a class-4
// Space/Enter/Esc key is popped; Esc on a menu whose header byte +0xe names an item (0x0053d999) not
// masked by +0x1f bits 0x30 marks that item and the selected one [0x0082fd45] (0x0053d98c) with bit
// 0x40, makes it the selection and latches 0x41b instead (otherwise the key is popped once more and
// nothing happens); the latched key, latch 1, countdown 0xa and the selected item's position (+8/+0xa
// words) start event 2. An event goes to edx and [0x0082fd9c], sets bit 1 of [0x0082fd6a], calls
// 0x0054ef3e and returns CF = 0; without one CF = 1 (other flags those of the last flag writer).

// Returns the event code (2, 3 or 5) or 0, leaves the flags of the last flag writer in `flags`.
static uint32_t key_step(gp4::Regs* r, uint32_t& flags) {
    gp4::call_regs(0x0052d7c8, *r);
    const uint32_t key = r->eax & 0xffu;
    const uint32_t kind = (r->eax >> 8) & 0xffu;

    if ((GP4_GLOBAL(uint8_t, 0x0082fd6b) & 1u) != 0u) {
        if (kind == 6u) {
            const uint32_t latched = GP4_GLOBAL(uint16_t, 0x007ab00e);
            r->edx = (r->edx & 0xffff0000u) | latched;
            if (key == (latched & 0xffu)) {
                gp4::call_regs(0x0052d7e2, *r);
                GP4_GLOBAL(uint8_t, 0x0082fd6b) = 0u;
                return 5u;
            }
        }
        if (kind != 0u) {
            gp4::call_regs(0x0052d7e2, *r);
        }
        const uint32_t countdown = GP4_GLOBAL(uint8_t, 0x007ab003);
        flags = gp4::flags::sub8(countdown, 0u);
        if (countdown != 0u) {
            return 0u;
        }
        GP4_GLOBAL(uint8_t, 0x007ab003) = 2u;
        const uint32_t latched_key = GP4_GLOBAL(uint8_t, 0x007ab00e);
        flags = gp4::flags::sub8(latched_key, 0x1bu);
        if (latched_key == 0x1bu) {
            return 0u;
        }
        const uint32_t x = GP4_GLOBAL(uint32_t, 0x0082fd7c);
        const uint32_t y = GP4_GLOBAL(uint32_t, 0x0082fd80);
        r->ebx = x;
        r->eax = y;
        GP4_GLOBAL(uint32_t, 0x0082fd94) = x;
        GP4_GLOBAL(uint32_t, 0x0082fd98) = y;
        return 3u;
    }

    flags = gp4::flags::sub8(kind, 4u);
    if (kind != 4u) {
        return 0u;
    }
    if (key != 0x20u) {
        flags = gp4::flags::sub8(key, 0x0du);
        if (key != 0x0du) {
            flags = gp4::flags::sub8(key, 0x1bu);
            if (key != 0x1bu) {
                return 0u;
            }
        }
    }

    gp4::call_regs(0x0052d7e2, *r);
    if ((r->eax & 0xffu) == 0x1bu) {
        const uint32_t header = GP4_GLOBAL(uint32_t, 0x0082fd34);
        r->esi = header;
        const uint32_t target = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(header + 0x0eu));
        r->eax = target;
        bool rejected = target == 0u;
        if (!rejected) {
            r->ecx = target;
            gp4::call_regs(0x0053d999, *r);
            const uint8_t* const item = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->edi));
            rejected = (item[0x1f] & 0x30u) != 0u;
        }
        if (rejected) {
            gp4::call_regs(0x0052d7e2, *r);
            flags = r->eflags;
            return 0u;
        }
        r->ecx = GP4_GLOBAL(uint8_t, 0x0082fd45);
        gp4::call_regs(0x0053d98c, *r);
        uint8_t* const selected = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
        uint8_t* const item = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
        selected[0x1f] = static_cast<uint8_t>(selected[0x1f] | 0x40u);
        item[0x1f] = static_cast<uint8_t>(item[0x1f] | 0x40u);
        GP4_GLOBAL(uint8_t, 0x0082fd45) = static_cast<uint8_t>(r->eax);
        r->eax = (r->eax & 0xffff0000u) | 0x041bu;
    }
    GP4_GLOBAL(uint16_t, 0x007ab00e) = static_cast<uint16_t>(r->eax);
    GP4_GLOBAL(uint8_t, 0x0082fd6b) = 1u;
    GP4_GLOBAL(uint8_t, 0x007ab003) = 0x0au;
    r->ecx = GP4_GLOBAL(uint8_t, 0x0082fd45);
    gp4::call_regs(0x0053d98c, *r);
    const uint8_t* const sel = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t x = *reinterpret_cast<const uint16_t*>(sel + 8);
    const uint32_t y = *reinterpret_cast<const uint16_t*>(sel + 0x0a);
    r->ebx = x;
    r->eax = y;
    GP4_GLOBAL(uint32_t, 0x0082fd7c) = x;
    GP4_GLOBAL(uint32_t, 0x0082fd80) = y;
    GP4_GLOBAL(uint32_t, 0x0082fd94) = x;
    GP4_GLOBAL(uint32_t, 0x0082fd98) = y;
    return 2u;
}

// FUNCTION: GP4 0x0053c600
void __cdecl FUN_0053c600(gp4::Regs* r) {
    uint32_t code = 0u;
    const uint32_t open = GP4_GLOBAL(uint8_t, 0x0082fd49);
    uint32_t flags = gp4::flags::sub8(open, 0xffu);
    if (open == 0xffu) {
        const uint32_t other = GP4_GLOBAL(uint8_t, 0x0082fd79);
        flags = gp4::flags::sub8(other, 0xffu);
        if (other == 0xffu) {
            code = key_step(r, flags);
        }
    }

    if (code != 0u) {
        r->edx = code;
        GP4_GLOBAL(uint32_t, 0x0082fd9c) = code;
        const uint32_t state = GP4_GLOBAL(uint8_t, 0x0082fd6a) | 2u;
        GP4_GLOBAL(uint8_t, 0x0082fd6a) = static_cast<uint8_t>(state);
        flags = gp4::flags::logic8(state);
        gp4::cdecl_<0x0054ef3e, void>();
        flags &= ~gp4::CF;
    } else {
        flags |= gp4::CF;
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0053c600, FUN_0053c600, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:u32, edi:u32) -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keyq_008245c0+menu_0084300a+state_0053d9a6+mouse_0053c500+sel_0053d8c5+key_0053c600")
