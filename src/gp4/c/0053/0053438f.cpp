#include <gp4/gp4.h>
#include <gp4/flags.h>

// Looks the control code byte up with 0x0052c689 (eax/ebx/ecx = a/b/c of the entry, ebx = 0 when unknown).
static inline void Lookup(gp4::Regs& c, uint8_t key) {
    c.eax = (c.eax & 0xffffff00u) | key;
    gp4::call_regs(0x0052c689, c);
}

// esi = the 16-byte record (a * 6 + c) of the table at 0x7ab02c, eax = its first dword.
static inline uint32_t Record(gp4::Regs& c) {
    c.esi = (c.eax * 6u + c.ecx) * 0x10u + 0x007ab02cu;
    c.eax = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(c.esi));
    return c.esi;
}

// 0x00534538 with ebx/ecx loaded: returns its CF.
static inline bool Compare(gp4::Regs& c, uint32_t ebx, uint32_t ecx) {
    c.ebx = ebx;
    c.ecx = ecx;
    gp4::call_regs(0x00534538, c);
    return (c.eflags & gp4::flags::CF) != 0u;
}

// Bit (1 << cl) of the mask 0x7a979c: eax = the bit, returns the flags of the `test`.
static inline uint32_t MaskBit(gp4::Regs& c) {
    c.eax = 1u << (c.ecx & 31u);
    return GP4_GLOBAL(uint32_t, 0x007a979c) & c.eax;
}

// Rebuilds the five state bits of 0x828235 (the old byte is saved at 0x828236) from the control codes
// 0x7ab54a, 0x7ab550, 0x7ab553, 0x7ab558 and 0x7ab559. Codes with a nonzero b compare the first dword
// of their 0x7ab02c record against the blends of two other values (0x00534538; the fixed globals
// 0x7ab134..0x7ab14c when byte 0x7ab0f1 is set); the others test a bit of the mask 0x7a979c.
// FUNCTION: GP4 0x0053438f
void __cdecl FUN_0053438f(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    uint8_t dl = GP4_GLOBAL(uint8_t, 0x00828235);
    GP4_GLOBAL(uint8_t, 0x00828236) = dl;
    const bool fixed_ = GP4_GLOBAL(uint8_t, 0x007ab0f1) != 0;

    Lookup(c, GP4_GLOBAL(uint8_t, 0x007ab54a));
    if (c.ebx != 0u) {
        const uint32_t rec = Record(c);
        const uint32_t* p = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(rec));
        bool cf;
        if (GP4_GLOBAL(uint8_t, 0x007ab0f1) != 0) {
            cf = Compare(c, GP4_GLOBAL(uint32_t, 0x007ab13c), GP4_GLOBAL(uint32_t, 0x007ab138));
        } else {
            cf = Compare(c, p[2], p[1]);
        }
        dl = cf ? (uint8_t)(dl | 1u) : (uint8_t)(dl & 0xfeu);
        if (GP4_GLOBAL(uint8_t, 0x007ab0f1) != 0) {
            cf = Compare(c, GP4_GLOBAL(uint32_t, 0x007ab13c), GP4_GLOBAL(uint32_t, 0x007ab134));
        } else {
            cf = Compare(c, p[2], p[3]);
        }
        dl = cf ? (uint8_t)(dl | 2u) : (uint8_t)(dl & 0xfdu);
    }
    (void)fixed_;

    Lookup(c, GP4_GLOBAL(uint8_t, 0x007ab550));
    if (c.ebx != 0u) {
        const uint32_t rec = Record(c);
        const uint32_t* p = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(rec));
        bool cf;
        if (GP4_GLOBAL(uint8_t, 0x007ab0f1) != 0) {
            cf = Compare(c, GP4_GLOBAL(uint32_t, 0x007ab144), GP4_GLOBAL(uint32_t, 0x007ab140));
        } else {
            cf = Compare(c, p[2], p[1]);
        }
        dl = cf ? (uint8_t)(dl | 4u) : (uint8_t)(dl & 0xfbu);
    } else {
        if (c.eax == 1u) {
            c.ecx += 0x10u;
        }
        dl = MaskBit(c) != 0u ? (uint8_t)(dl | 4u) : (uint8_t)(dl & 0xfbu);
    }

    Lookup(c, GP4_GLOBAL(uint8_t, 0x007ab553));
    if (c.ebx != 0u) {
        const uint32_t rec = Record(c);
        const uint32_t* p = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(rec));
        bool cf;
        if (GP4_GLOBAL(uint8_t, 0x007ab0f1) != 0) {
            cf = Compare(c, GP4_GLOBAL(uint32_t, 0x007ab14c), GP4_GLOBAL(uint32_t, 0x007ab148));
        } else {
            cf = Compare(c, p[2], p[3]);
        }
        dl = cf ? (uint8_t)(dl | 8u) : (uint8_t)(dl & 0xf7u);
    } else {
        if (c.eax == 1u) {
            c.ecx += 0x10u;
        }
        dl = MaskBit(c) != 0u ? (uint8_t)(dl | 8u) : (uint8_t)(dl & 0xf7u);
    }
    dl = (uint8_t)(dl & 0xefu);

    Lookup(c, GP4_GLOBAL(uint8_t, 0x007ab558));
    if (c.ebx == 0u) {
        if (MaskBit(c) != 0u) {
            dl = (uint8_t)(dl | 0x10u);
        }
    }

    uint32_t arith;
    Lookup(c, GP4_GLOBAL(uint8_t, 0x007ab559));
    arith = gp4::flags::logic32(c.ebx);
    if (c.ebx == 0u) {
        const uint32_t hit = MaskBit(c);
        arith = gp4::flags::logic32(hit);
        if (hit != 0u) {
            dl = (uint8_t)(dl | 0x10u);
            arith = gp4::flags::logic8(dl);
        }
    }
    GP4_GLOBAL(uint8_t, 0x00828235) = dl;

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = (c.edx & 0xffffff00u) | dl;
    r->esi = c.esi;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0053438f, FUN_0053438f, "regs(edx:u32, esi:u32) -> (eax, ebx, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ctl_007ab548")
