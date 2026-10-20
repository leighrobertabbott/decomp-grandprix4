#include <gp4/gp4.h>
#include <gp4/flags.h>

// For the current object index [0x679bd8] (0..31), picks seven dwords: each of the parallel
// 32-entry tables 0x6f33ec/0x6f34ec/0x6f346c/0x6f356c/0x6f35ec/0x6f366c/0x6f36ec overrides its
// default 0x6f396c/0x6f3974/0x6f3970/0x6f3978/0x6f397c/0x6f3980/0x6f3984 when its entry is
// nonzero, and the result goes to 0x6f3988/0x6f3990/0x6f398c/0x6f3994/0x6f3998/0x6f399c/0x6f39a0.
// ebx ends as the index, eax as the last value, flags from the last `test entry, -1`.
// FUNCTION: GP4 0x00525595
void __cdecl FUN_00525595(gp4::Regs* r) {
    const uint32_t index = GP4_GLOBAL(uint32_t, 0x00679bd8);
    uint32_t value;
    uint32_t entry;

    value = GP4_GLOBAL(uint32_t, 0x006f396c);
    entry = GP4_ARRAY(uint32_t, 0x006f33ec)[index];
    if (entry != 0u) value = entry;
    GP4_GLOBAL(uint32_t, 0x006f3988) = value;

    value = GP4_GLOBAL(uint32_t, 0x006f3974);
    entry = GP4_ARRAY(uint32_t, 0x006f34ec)[index];
    if (entry != 0u) value = entry;
    GP4_GLOBAL(uint32_t, 0x006f3990) = value;

    value = GP4_GLOBAL(uint32_t, 0x006f3970);
    entry = GP4_ARRAY(uint32_t, 0x006f346c)[index];
    if (entry != 0u) value = entry;
    GP4_GLOBAL(uint32_t, 0x006f398c) = value;

    value = GP4_GLOBAL(uint32_t, 0x006f3978);
    entry = GP4_ARRAY(uint32_t, 0x006f356c)[index];
    if (entry != 0u) value = entry;
    GP4_GLOBAL(uint32_t, 0x006f3994) = value;

    value = GP4_GLOBAL(uint32_t, 0x006f397c);
    entry = GP4_ARRAY(uint32_t, 0x006f35ec)[index];
    if (entry != 0u) value = entry;
    GP4_GLOBAL(uint32_t, 0x006f3998) = value;

    value = GP4_GLOBAL(uint32_t, 0x006f3980);
    entry = GP4_ARRAY(uint32_t, 0x006f366c)[index];
    if (entry != 0u) value = entry;
    GP4_GLOBAL(uint32_t, 0x006f399c) = value;

    value = GP4_GLOBAL(uint32_t, 0x006f3984);
    entry = GP4_ARRAY(uint32_t, 0x006f36ec)[index];
    if (entry != 0u) value = entry;
    GP4_GLOBAL(uint32_t, 0x006f39a0) = value;

    r->eax = value;
    r->ebx = index;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(entry));
}
GP4_IMPL(0x00525595, FUN_00525595, "regs() -> (eax, ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ptrtab_00524fa4+tabs_00525595")
