#include <gp4/gp4.h>

// Four 0x300-byte records at 0x006380d8 (only the fields this routine touches).
struct Unknown_006380d8 {
    uint8_t Unknown000[0x2e];
    uint8_t Flags02e;                     // +0x2e (bit 0x40 set here)
    uint8_t Unknown02f[0x6c - 0x2f];
    int32_t Unknown06c;                   // +0x6c
    uint8_t Unknown070[0x74 - 0x70];
    int32_t Unknown074;                   // +0x74 (Q14 blend factor)
    uint8_t Unknown078[0x8c - 0x78];
    int32_t Unknown08c;                   // +0x8c
    int32_t Unknown090;                   // +0x90 (upper clamp)
    uint8_t Unknown094[0xb0 - 0x94];
    int32_t Unknown0b0;                   // +0xb0
    uint8_t Unknown0b4[0xbc - 0xb4];
    int32_t Unknown0bc;                   // +0xbc
    uint8_t Unknown0c0[0xc4 - 0xc0];
    int32_t Unknown0c4;                   // +0xc4
    uint8_t Unknown0c8[0x114 - 0xc8];
    int32_t Unknown114;
    int32_t Unknown118;
    int32_t Unknown11c;
    int32_t Unknown120;
    int32_t Unknown124;
    int32_t Unknown128;
    int32_t Unknown12c;
    int32_t Unknown130;
    int32_t Unknown134;
    int32_t Unknown138;
    int32_t Unknown13c;
    int32_t Unknown140;
    int32_t Unknown144;
    int32_t Unknown148;
    int32_t Unknown14c;
    int32_t Unknown150;
    uint8_t Unknown154[0x15c - 0x154];
    int32_t Unknown15c;                   // +0x15c
    uint8_t Unknown160[0x178 - 0x160];
    int32_t Unknown178;                   // +0x178
    uint8_t Unknown17c[0x1c8 - 0x17c];
    int32_t Unknown1c8;                   // +0x1c8
    uint8_t Unknown1cc[0x1d0 - 0x1cc];
    int32_t Unknown1d0;                   // +0x1d0
    int32_t Unknown1d4;                   // +0x1d4
    uint8_t Unknown1d8[0x2c4 - 0x1d8];
    int32_t Unknown2c4;                   // +0x2c4
    uint8_t Unknown2c8[0x300 - 0x2c8];
};
static_assert(sizeof(Unknown_006380d8) == 0x300, "record stride");

// The esi block: per-wheel arrays of four dwords (wheel k at +4k).
struct Unknown_0041f805_esi {
    uint8_t Unknown000[0xcf];
    uint8_t Flags0cf;                     // +0xcf (bit 0x10 selects the limited mode)
    uint8_t Unknown0d0[0x2cc - 0xd0];
    int32_t Unknown2cc[4];
    uint8_t Unknown2dc[0x2fc - 0x2dc];
    int32_t Unknown2fc[4];
    uint8_t Unknown30c[0x31c - 0x30c];
    int32_t Unknown31c[4];
    int32_t Unknown32c[4];
    int32_t Unknown33c[4];
    int32_t Unknown34c[4];
    int32_t Unknown35c[4];
    uint8_t Unknown36c[0x37c - 0x36c];
    int32_t Unknown37c[4];
    uint8_t Unknown38c[0x3ac - 0x38c];
    int32_t Unknown3ac[4];
    int32_t Unknown3bc[4];
    int32_t Unknown3cc[4];
    int32_t Unknown3dc[4];
};
static_assert(sizeof(Unknown_0041f805_esi) == 0x3ec, "esi block");

// imul (signed 32x32 -> 64) then shrd 14: low dword of the product >> 14.
static inline uint32_t mulShr14(uint32_t a, int32_t b) {
    return (uint32_t)((uint64_t)((int64_t)(int32_t)a * (int64_t)b) >> 14);
}

// FUNCTION: GP4 0x0041f805
void __cdecl FUN_0041f805(gp4::Regs* r) {
    Unknown_0041f805_esi* const car = (Unknown_0041f805_esi*)r->esi;
    Unknown_006380d8* const rec = GP4_ARRAY(Unknown_006380d8, 0x006380d8);

    GP4_GLOBAL(uint8_t, 0x00601f1f) = (car->Flags0cf & 0x10) ? 0xff : 0x00;

    // Records 0/1 from wheels 0/1, records 2/3 from wheels 2/3 blended toward 0/1.
    const uint32_t base01 = (uint32_t)car->Unknown2fc[0] - (uint32_t)car->Unknown37c[0];
    const uint32_t span01 = ((uint32_t)car->Unknown2fc[1] - (uint32_t)car->Unknown37c[1]) - base01;
    rec[0].Unknown0b0 = (int32_t)(mulShr14(span01, rec[0].Unknown074) + base01);
    rec[1].Unknown0b0 = (int32_t)(mulShr14(span01, rec[1].Unknown074) + base01);

    const uint32_t base23 = (uint32_t)car->Unknown2fc[2] - (uint32_t)car->Unknown37c[2];
    const uint32_t span23 = ((uint32_t)car->Unknown2fc[3] - (uint32_t)car->Unknown37c[3]) - base23;
    const int32_t blend = GP4_GLOBAL(int32_t, 0x00601e3c);
    uint32_t v = mulShr14(span23, rec[2].Unknown074) + base23;
    rec[2].Unknown0b0 = (int32_t)(v + mulShr14((uint32_t)rec[0].Unknown0b0 - v, blend));
    v = mulShr14(span23, rec[3].Unknown074) + base23;
    rec[3].Unknown0b0 = (int32_t)(v + mulShr14((uint32_t)rec[1].Unknown0b0 - v, blend));

    GP4_GLOBAL(uint8_t, 0x00601ea8) = 0;
    GP4_GLOBAL(uint8_t, 0x00601eaa) = 0;
    GP4_GLOBAL(int32_t, 0x00601d3c) = 0;

    uint32_t eax = 0;
    uint32_t edx = 0;
    for (int k = 0; k < 4; ++k) {
        Unknown_006380d8* const w = &rec[k];
        w->Unknown124 = 0;
        w->Unknown128 = 0;
        w->Unknown12c = 0;
        w->Unknown130 = 0;

        const int32_t load = (int32_t)(0u - (uint32_t)car->Unknown34c[k]);
        w->Unknown118 = load;
        if (GP4_GLOBAL(uint8_t, 0x00601f1f) != 0xff) {
            if (load < 0) w->Unknown120 = car->Unknown3cc[k];
            else w->Unknown120 = car->Unknown3ac[k];
        } else if (load >= 0) {
            const int32_t lim = GP4_GLOBAL(int32_t, 0x00601e50);
            if (load > lim) {
                w->Unknown120 = car->Unknown3bc[k];
                w->Unknown128 = lim;
                w->Unknown130 = (int32_t)((uint32_t)car->Unknown3ac[k] - (uint32_t)car->Unknown3bc[k]);
            } else {
                w->Unknown120 = car->Unknown3ac[k];
            }
        } else {
            const int32_t lim = GP4_GLOBAL(int32_t, 0x00601e54);
            if (load < lim) {
                w->Unknown120 = car->Unknown3dc[k];
                w->Unknown128 = lim;
                w->Unknown130 = (int32_t)((uint32_t)car->Unknown3cc[k] - (uint32_t)car->Unknown3dc[k]);
            } else {
                w->Unknown120 = car->Unknown3cc[k];
            }
        }

        int32_t over = (int32_t)((uint32_t)w->Unknown0b0 - (uint32_t)(w->Unknown178 >> 16)
                                 - (uint32_t)w->Unknown2c4);
        if (over < 0) {
            GP4_GLOBAL(uint8_t, 0x00601ea8) = 0xff;
            GP4_GLOBAL(uint8_t, 0x00625abb) = 0xff;
            w->Flags02e |= 0x40;
            over = (int32_t)(0u - (uint32_t)over);
        } else {
            over = 0;
        }
        w->Unknown0bc = over;

        const int32_t pos = car->Unknown32c[k];
        if (pos < 0) {
            w->Unknown114 = (int32_t)(0u - (uint32_t)pos);
            w->Unknown11c = (int32_t)((uint32_t)GP4_GLOBAL(int32_t, 0x00601df0) + (uint32_t)car->Unknown2cc[k]);
        } else {
            const int32_t room = (int32_t)((uint32_t)w->Unknown1c8 - (uint32_t)car->Unknown35c[k]);
            if (pos >= room) {
                GP4_GLOBAL(uint8_t, 0x00601eaa) = 0xff;
                w->Unknown114 = (int32_t)(0u - (uint32_t)pos);
                const int32_t t = w->Unknown1d0;
                w->Unknown12c = t;
                w->Unknown11c = (int32_t)((uint32_t)t + (uint32_t)car->Unknown2cc[k]);
                w->Unknown124 = room;
            } else {
                GP4_GLOBAL(int32_t, 0x00601d3c) += 1;
                w->Unknown114 = (int32_t)(0u - (uint32_t)pos);
                w->Unknown11c = car->Unknown2cc[k];
            }
        }

        const uint32_t p = (uint32_t)car->Unknown32c[k];
        int32_t gap = (int32_t)((uint32_t)w->Unknown08c + (uint32_t)w->Unknown1d4 - p
                                - (uint32_t)car->Unknown33c[k] - (uint32_t)car->Unknown2fc[k]
                                + (uint32_t)car->Unknown37c[k]);
        if (gap > w->Unknown090) gap = w->Unknown090;
        if (gap < 0) {
            car->Unknown31c[k] = 0;
            w->Unknown144 = 0;
            w->Unknown148 = 0;
            w->Unknown14c = 0;
            w->Unknown150 = 0;
            w->Unknown134 = 0;
            w->Unknown138 = 0;
            w->Unknown13c = 0;
            w->Unknown140 = 0;
            eax = (uint32_t)gap;
            edx = p;
        } else {
            car->Unknown31c[k] = gap;
            w->Unknown144 = (int32_t)((uint32_t)gap + p);
            w->Unknown134 = (int32_t)(0u - p);
            const int32_t a = GP4_GLOBAL(int32_t, 0x00601dac);
            w->Unknown14c = a;
            w->Unknown13c = a;
            const uint32_t ref = (uint32_t)w->Unknown06c + (uint32_t)(w->Unknown15c >> 4);
            const int32_t diff = (int32_t)((uint32_t)car->Unknown34c[k] + (uint32_t)w->Unknown0c4 - ref);
            if (diff >= 0) {
                w->Unknown148 = 0;
                w->Unknown150 = 0;
                w->Unknown140 = 0;
                w->Unknown138 = 0;
                eax = ref;
            } else {
                w->Unknown148 = (int32_t)(ref - (uint32_t)w->Unknown0c4);
                w->Unknown138 = (int32_t)(0u - (uint32_t)car->Unknown34c[k]);
                const int32_t b = GP4_GLOBAL(int32_t, 0x00601db4);
                w->Unknown150 = b;
                w->Unknown140 = b;
                eax = (uint32_t)b;
            }
            edx = (uint32_t)diff;
        }
    }

    // Loop exit: cmp ebx,0x638cd8 equal -> ZF=PF=1, CF=AF=SF=OF=0; esi/ebx popped.
    r->eax = eax;
    r->ecx = base23;
    r->edx = edx;
    r->ebx = 0x006380d8u;
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0041f805, FUN_0041f805, "regs(esi:ptr[0x3f8]:bytes) -> (eax, ecx, edx, ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_004178e0+integ_004200a7+wheel_004205a0+lim_00420e6b")
