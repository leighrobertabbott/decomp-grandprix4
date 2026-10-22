#include <gp4/gp4.h>
#include <gp4/flags.h>

// One helper per register-routine target so every call site stays a real call.
static __declspec(noinline) void CallCommand(gp4::Regs& c) {
    gp4::call_regs(0x00531f9c, c);
}
static __declspec(noinline) void CallHsvToRgb(gp4::Regs& c) {
    gp4::call_regs(0x0051b2ea, c);
}
static __declspec(noinline) void CallNearest(gp4::Regs& c) {
    gp4::call_regs(0x0051b645, c);
}
static __declspec(noinline) void CallBrightRows(gp4::Regs& c) {
    gp4::call_regs(0x0051be22, c);
}
static __declspec(noinline) void CallRemapTables(gp4::Regs& c) {
    gp4::call_regs(0x0051bc7b, c);
}

// pushal wrapper building the 16 shaded rows of the 256-colour remap table 0x923104. The search
// flags 0x954060..0x95415f are set to 0xff (indices 0/1 cleared) with the restricted-search byte
// [0x95405e] = 0xff; the point table [0x96a3d4] = 0x7c4d5c and the hue/saturation/value table
// [0x96a3d0] = 0x7c535c. For each row (scale 0x1a80..0x4000 step 0x280) command 0x41 runs through
// 0x531f9c, then for colours 255..2 the entry's hue/saturation and value * (scale >> 8) >> 6 go to
// 0x933a1a/1c/1e, 0x51b2ea converts them and 0x51b645 picks the nearest colour (colours 1/0 map to
// themselves). 0x51be22 adds the brightened rows, the first row is copied ten times to 0x922704,
// [0x95405e] is cleared and 0x51bc7b runs when [0x7ad869] is nonzero. popal restores every
// register; the flags are those of the last writer (`cmp byte [0x7ad869], 0` or 0x51bc7b).
// FUNCTION: GP4 0x0051bd23
void __cdecl FUN_0051bd23(gp4::Regs* r) {
    gp4::Regs c = *r;
    GP4_GLOBAL(uint8_t, 0x0095405e) = 0xffu;
    uint8_t* const flagsTable = GP4_ARRAY(uint8_t, 0x00954060);
    for (uint32_t i = 0u; i != 0x100u; ++i) {
        flagsTable[i] = 0xffu;
    }
    c.eax = 0x100u;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(0x100u, 0x100u));
    GP4_GLOBAL(uint16_t, 0x00954060) = 0u;
    GP4_GLOBAL(uint32_t, 0x0096a3d4) = 0x007c4d5cu;
    GP4_GLOBAL(uint32_t, 0x0096a3d0) = 0x007c535cu;
    const uint32_t table = GP4_GLOBAL(uint32_t, 0x0096a3d0);
    c.esi = table;
    c.edi = 0u;
    c.ebp = 0x1a80u;
    do {
        const uint32_t saved = c.eax;
        c.eax = 0x41u;
        CallCommand(c);
        c.eax = saved;
        c.ecx = 0xffu;
        do {
            c.eax = (c.eax & 0xffffff00u) | (c.ecx & 0xffu);
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.ecx, 2u));
            if (static_cast<int32_t>(c.ecx) >= 2) {
                const uint16_t* const entry =
                    reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(c.ecx * 6u + table));
                GP4_GLOBAL(uint16_t, 0x00933a1a) = entry[0];
                GP4_GLOBAL(uint16_t, 0x00933a1c) = entry[1];
                const uint32_t scale = c.ebp >> 8;
                const uint32_t product = static_cast<uint32_t>(entry[2]) * (scale & 0xffffu);
                c.edx = (scale & 0xffff0000u) | (product >> 16);
                c.eax = static_cast<uint32_t>(static_cast<int32_t>(product) >> 6);
                GP4_GLOBAL(uint16_t, 0x00933a1e) = static_cast<uint16_t>(c.eax);
                CallHsvToRgb(c);
                CallNearest(c);
            }
            GP4_ARRAY(uint8_t, 0x00923104)[c.ecx + c.edi] = static_cast<uint8_t>(c.eax);
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::dec32(c.ecx, c.eflags));
            c.ecx -= 1u;
        } while (static_cast<int32_t>(c.ecx) >= 0);
        c.edi += 0x100u;
        c.ebp += 0x280u;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.ebp, 0x4000u));
    } while (c.ebp <= 0x4000u);

    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.edi, 0x1000u));
    CallBrightRows(c);

    uint32_t* dst = GP4_ARRAY(uint32_t, 0x00922704);
    for (uint32_t copy = 0u; copy != 10u; ++copy) {
        const uint32_t* src = GP4_ARRAY(uint32_t, 0x00923104);
        for (uint32_t n = 0u; n != 0x40u; ++n) {
            *dst++ = *src++;
        }
    }
    c.ecx = 0u;
    c.esi = 0x00923204u;
    c.edi = 0x00923104u;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::dec32(1u, c.eflags));
    c.ebp = 0u;
    GP4_GLOBAL(uint8_t, 0x0095405e) = 0u;

    const uint8_t remap = GP4_GLOBAL(uint8_t, 0x007ad869);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(remap, 0u));
    if (remap != 0u) {
        CallRemapTables(c);
    }
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051bd23, FUN_0051bd23, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=dw_00532d48; maxinsn=80000000; wall=3600")
