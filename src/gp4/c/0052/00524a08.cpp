#include <gp4/gp4.h>
#include <gp4/flags.h>

// Register routine called by 0x00524b3d with esi = [0x00679d30] (the selected car record at
// 0x0066fef0, stride 0x41c) and cl = [0x0066d7a4]. The key byte 0x0066d7a6 selects one action
// (tested in the order of bits 0, 4, 3, 5, 2, 1); the default walks the order list 0x0066fe3c
// forward (bit 7 clear) or backward (bit 7 set) from the car's entry offset +0x7d, wrapping at
// 0x2c, for at most [0x00679bce] steps, until a car whose +0x7a has neither 0x20 nor 0x80 and
// whose +0x8d has no 0x40. Success sets 0x006f3b7a to 0xff and returns CF clear; failure
// returns CF set. Both keep the other flags of the last flag writer.
// FUNCTION: GP4 0x00524a08
static inline void Done(gp4::Regs* r, uint32_t flags) {
    GP4_GLOBAL(uint8_t, 0x006f3b7a) = 0xffu;
    r->eflags = gp4::flags::merge(r->eflags, flags & ~1u);
}

static inline void Fail(gp4::Regs* r, uint32_t flags) {
    r->eflags = gp4::flags::merge(r->eflags, flags | 1u);
}

static inline void SetCl(gp4::Regs* r, uint8_t cl) {
    r->ecx = (r->ecx & ~0xffu) | cl;
}

static inline uint32_t SessionFlags() {
    const uint32_t a = GP4_GLOBAL(uint32_t, 0x00a53ad0);
    if (a == 0u) {
        return gp4::flags::logic32(a);
    }
    return gp4::flags::sub32(GP4_GLOBAL(uint32_t, 0x00645950), 1u);
}

void __cdecl FUN_00524a08(gp4::Regs* r) {
    const uint8_t key = GP4_GLOBAL(uint8_t, 0x0066d7a6);
    if ((key & 0x01u) != 0u) {
        gp4::call_regs(0x005249ca, *r);
        SetCl(r, 0xc0u);
        Done(r, r->eflags);
        return;
    }
    if ((key & 0x10u) != 0u) {
        const uint8_t bit = (uint8_t)(((const uint8_t*)r->esi)[0x7a] & 0x20u);
        const uint32_t flags = gp4::flags::logic8(bit);
        if (bit != 0u) {
            Fail(r, flags);
            return;
        }
        GP4_GLOBAL(uint32_t, 0x00679bdc) = 0u;
        SetCl(r, 0u);
        Done(r, flags);
        return;
    }
    if ((key & 0x08u) != 0u) {
        const uint32_t sel = GP4_GLOBAL(uint32_t, 0x0066d820);
        r->eax = sel;
        const uint32_t flags = gp4::flags::logic32(sel);
        if (sel == 0u) {
            Fail(r, flags);
            return;
        }
        r->esi = sel;
        Done(r, flags);
        return;
    }
    if ((key & 0x20u) != 0u) {
        r->eflags = gp4::flags::merge(r->eflags, SessionFlags());
        gp4::call_regs(0x00525270, *r);
        SetCl(r, 0x80u);
        Done(r, r->eflags);
        return;
    }
    if ((key & 0x04u) != 0u) {
        const uint32_t flags = SessionFlags();
        SetCl(r, 0xa0u);
        Done(r, flags);
        return;
    }
    if ((key & 0x02u) != 0u) {
        const uint32_t flags = SessionFlags();
        SetCl(r, 0xb0u);
        Done(r, flags);
        return;
    }
    const uint8_t* car = (const uint8_t*)r->esi;
    const uint8_t* list = GP4_ARRAY(const uint8_t, 0x0066fe3c);
    for (;;) {
        uint16_t index = car[0x7d];
        if ((GP4_GLOBAL(uint8_t, 0x0066d7a6) & 0x80u) != 0u) {
            index = (uint16_t)(index - 2u);
            if ((int16_t)index < 0) {
                index = 0x2au;
            }
        } else {
            index = (uint16_t)(index + 2u);
            if ((int16_t)index >= 0x2c) {
                index = 0u;
            }
        }
        const uint16_t offset = *(const uint16_t*)(list + (int16_t)index);
        const uint32_t delta = (uint32_t)(int32_t)(int16_t)offset;
        r->ebx = 0x0066fe3cu;
        r->eax = (uint32_t)offset;
        r->esi = delta + 0x0066fef0u;
        car = (const uint8_t*)r->esi;
        uint32_t flags;
        uint8_t bit = (uint8_t)(car[0x7a] & 0x20u);
        flags = gp4::flags::logic8(bit);
        if (bit == 0u) {
            bit = (uint8_t)(car[0x7a] & 0x80u);
            flags = gp4::flags::logic8(bit);
            if (bit == 0u) {
                bit = (uint8_t)(car[0x8d] & 0x40u);
                flags = gp4::flags::logic8(bit);
                if (bit == 0u) {
                    Done(r, flags);
                    return;
                }
            }
        }
        const uint16_t count = GP4_GLOBAL(uint16_t, 0x00679bce);
        flags = gp4::flags::sub16(count, 1u);
        GP4_GLOBAL(uint16_t, 0x00679bce) = (uint16_t)(count - 1u);
        if ((uint16_t)(count - 1u) == 0u) {
            Fail(r, flags);
            return;
        }
    }
}
GP4_IMPL(0x00524a08, FUN_00524a08, "regs(ecx:u32, esi:rec[0x66fef0,0x41c,0..21]) -> (eax, ebx, ecx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+key_00524a08")
