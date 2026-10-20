#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal routine answering in CF. Only when the mode byte 0x007c2ea9 has bit 7 set
// (else stc, flags of its test): clears the flags 0x00866058 / 0x0086605c, and when the master
// byte 0x0066d7a4 is zero (else stc) looks at the cars next to the viewed car [0x00679d30]:
//   0x0040eaf7 (car found through the order table 0x7c31db), checked by 0x0040eb1f (ebx = found,
//   esi = viewed): accepted -> [0x866060] = its eax, [0x866058] = 1, [0x866068] = driver name entry
//   of the found car's byte +0x7c (0x0051f2af), 0x0051d160, edi = -1, byte 0x006f2c34 = 0,
//   event [0x0088ff30] = 0xc through 0x00514152 (esi = found car), then the found car's dword
//   +0x4c = [0x0066d814] + 4000, bit 3 of its byte +0x89 cleared, byte 0x00954169 = 0xff;
//   0x0040eb0b (the other neighbour), checked by 0x0040eb1f with the roles swapped (esi = found):
//   accepted -> [0x866064], [0x86605c] = 1, [0x86606c] = name entry, 0x0051d160 only if the first
//   car did not run it, then the same event 0xc / +0x4c / +0x89 / 0x954169 updates.
// clc when either car was accepted, else stc (other flags of `or edi, edi`). popal restores every
// GPR; registers flow through the callees as in the original.
// FUNCTION: GP4 0x00514584
void __cdecl FUN_00514584(gp4::Regs* r) {
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    if ((mode & 0x80u) == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(mode) | gp4::flags::CF);
        return;
    }
    GP4_GLOBAL(uint32_t, 0x00866058) = 0u;
    GP4_GLOBAL(uint32_t, 0x0086605c) = 0u;
    const uint8_t master = GP4_GLOBAL(uint8_t, 0x0066d7a4);
    if (master != 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(master) | gp4::flags::CF);
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    c.edi = 0u;
    c.esi = GP4_GLOBAL(uint32_t, 0x00679d30);
    gp4::call_regs(0x0040eaf7, c);
    c.ebx = c.eax;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.ebx));
    if (c.ebx != 0u) {
        gp4::call_regs(0x0040eb1f, c);
        if ((c.eflags & gp4::flags::CF) == 0u) {
            GP4_GLOBAL(uint32_t, 0x00866060) = c.eax;
            GP4_GLOBAL(uint32_t, 0x00866058) = 1u;
            c.esi = c.ebx;
            uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.esi));
            c.eax = (c.eax & 0xffffff00u) | car[0x7c];
            gp4::call_regs(0x0051f2af, c);
            GP4_GLOBAL(uint32_t, 0x00866068) = c.eax;
            gp4::call_regs(0x0051d160, c);
            c.edi = 0xffffffffu;
            GP4_GLOBAL(uint8_t, 0x006f2c34) = 0u;
            GP4_GLOBAL(uint32_t, 0x0088ff30) = 0xcu;
            gp4::call_regs(0x00514152, c);
            car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.esi));
            GP4_FIELD(uint32_t, car, 0x4c) = GP4_GLOBAL(uint32_t, 0x0066d814) + 0xfa0u;
            const uint8_t b = static_cast<uint8_t>(car[0x89] & 0xf7u);
            car[0x89] = b;
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(b));
            GP4_GLOBAL(uint8_t, 0x00954169) = 0xffu;
        }
    }

    c.esi = GP4_GLOBAL(uint32_t, 0x00679d30);
    gp4::call_regs(0x0040eb0b, c);
    c.ebx = c.eax;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.ebx));
    if (c.ebx != 0u) {
        uint32_t t = c.ebx;
        c.ebx = c.esi;
        c.esi = t;
        gp4::call_regs(0x0040eb1f, c);
        t = c.ebx;
        c.ebx = c.esi;
        c.esi = t;
        if ((c.eflags & gp4::flags::CF) == 0u) {
            GP4_GLOBAL(uint32_t, 0x00866064) = c.eax;
            GP4_GLOBAL(uint32_t, 0x0086605c) = 1u;
            c.esi = c.ebx;
            uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.esi));
            c.eax = (c.eax & 0xffffff00u) | car[0x7c];
            gp4::call_regs(0x0051f2af, c);
            GP4_GLOBAL(uint32_t, 0x0086606c) = c.eax;
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.edi));
            if (c.edi == 0u) {
                gp4::call_regs(0x0051d160, c);
                c.edi = 0xffffffffu;
            }
            GP4_GLOBAL(uint32_t, 0x0088ff30) = 0xcu;
            gp4::call_regs(0x00514152, c);
            car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.esi));
            GP4_FIELD(uint32_t, car, 0x4c) = GP4_GLOBAL(uint32_t, 0x0066d814) + 0xfa0u;
            const uint8_t b = static_cast<uint8_t>(car[0x89] & 0xf7u);
            car[0x89] = b;
            GP4_GLOBAL(uint8_t, 0x00954169) = 0xffu;
        }
    }

    const uint32_t carry = c.edi == 0u ? gp4::flags::CF : 0u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(c.edi) | carry);
}
GP4_IMPL(0x00514584, FUN_00514584, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:u32, edi:u32, ebp:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carno_0066ff6c+cars_004362a9+due_0066ffb0+player_0040f8d9+disp_005141b8+events_00514152+pair_00514584")
