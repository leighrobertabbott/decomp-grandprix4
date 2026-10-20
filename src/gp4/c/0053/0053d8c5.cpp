#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0053d8c5
// Menu navigation: moves the selection (byte 0x0082fd45, 1-based index into the item chain that
// starts at [[0x0082fd34]]) backwards to the previous item that can be selected, wrapping to the
// item count dword 0x0082fd52, and stops when the walk is back at the current selection. An item
// qualifies when its flag byte +0x1f has neither bit 0x10 nor 0x20, its type byte +0xd has both
// low bits set and its byte +0x1e has bit 0x40. On success bit 0x40 of +0x1f is set on both
// the item in esi (the previous one) and the new one (edi) and the selection byte is updated.
// edi comes back from 0x0053d999 (the item reached by `ecx` hops); al ends as the last type&3.
void __cdecl FUN_0053d8c5(gp4::Regs* r) {
    uint32_t eax = r->eax;
    uint32_t edi = r->edi;
    const uint32_t esi = r->esi;
    uint32_t ecx = GP4_GLOBAL(uint8_t, 0x0082fd45);
    uint32_t flags;
    uint32_t al = eax & 0xffu;

    for (;;) {
        ecx -= 1u;
        if (ecx == 0u) {
            ecx = GP4_GLOBAL(uint32_t, 0x0082fd52);
        }
        flags = gp4::flags::sub8(ecx & 0xffu, GP4_GLOBAL(uint8_t, 0x0082fd45));
        if ((ecx & 0xffu) == GP4_GLOBAL(uint8_t, 0x0082fd45)) {
            break;
        }

        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.ecx = ecx;
        gp4::call_regs(0x0053d999, c);
        edi = c.edi;

        uint8_t* item = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(edi));
        if ((GP4_FIELD(uint8_t, item, 0x1f) & 0x10u) != 0u) {
            continue;
        }
        if ((GP4_FIELD(uint8_t, item, 0x1f) & 0x20u) != 0u) {
            continue;
        }
        al = GP4_FIELD(uint8_t, item, 0xd) & 3u;
        if (al != 3u) {
            continue;
        }
        if ((GP4_FIELD(uint8_t, item, 0x1e) & 0x40u) == 0u) {
            continue;
        }

        uint8_t* previous = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(esi));
        GP4_FIELD(uint8_t, previous, 0x1f) = static_cast<uint8_t>(GP4_FIELD(uint8_t, previous, 0x1f) | 0x40u);
        const uint8_t updated = static_cast<uint8_t>(GP4_FIELD(uint8_t, item, 0x1f) | 0x40u);
        GP4_FIELD(uint8_t, item, 0x1f) = updated;
        flags = gp4::flags::logic8(updated);
        GP4_GLOBAL(uint8_t, 0x0082fd45) = static_cast<uint8_t>(ecx);
        break;
    }

    r->eax = (eax & 0xffffff00u) | al;
    r->ecx = ecx;
    r->edi = edi;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0053d8c5, FUN_0053d8c5, "regs(eax:u32, edi:u32, esi:ptr[0x40]:bytes) -> (eax, ecx, edi, cf, pf, af, zf, sf, of); globals=layout=menu_0084300a+sel_0053d8c5")
