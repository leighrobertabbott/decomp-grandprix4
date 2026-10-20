#include <gp4/gp4.h>
#include <gp4/flags.h>

// The compiler would fold `address - address` into a constant and hide both globals.
static __declspec(noinline) uint32_t addrSpan(const uint8_t* end, const uint8_t* start) {
    return (uint32_t)(end - start);
}

// pushal ... popal routine: refreshes the display-mode derived globals (0x7a8ea2/a6/aa/9a/9e from
// the mode queries 0x504c30..0x504cf0 on device [0x7ad87c], the frame sizes 0x829930/0x7a8e7a/
// 0x829980), the block size 0x829958 (0x504d10), re-runs the block sizer 0x5375f0 (and 0x537508
// when the block size [0x8299a8] changed) and stores the 0x833ff0 table entries + 2. Every GPR
// is restored; the flags are those of the final `add eax, 2`.
// FUNCTION: GP4 0x00538a96
void __cdecl FUN_00538a96(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x00512c27, c);
    GP4_GLOBAL(uint32_t, 0x008298b8) = GP4_GLOBAL(uint32_t, 0x007a93d8);
    gp4::cdecl_<0x00530774, uint32_t>();

    GP4_GLOBAL(uint32_t, 0x007a8ea2) = gp4::stdcall_<0x00504c30, uint32_t>(GP4_GLOBAL(uint32_t, 0x007ad87c));
    GP4_GLOBAL(uint32_t, 0x007a8ea6) = gp4::stdcall_<0x00504c40, uint32_t>(GP4_GLOBAL(uint32_t, 0x007ad87c));
    GP4_GLOBAL(uint32_t, 0x007a8eaa) = gp4::stdcall_<0x00504c50, uint32_t>(GP4_GLOBAL(uint32_t, 0x007ad87c));
    GP4_GLOBAL(uint32_t, 0x007a8e9a) = gp4::stdcall_<0x00504c60, uint32_t>(GP4_GLOBAL(uint32_t, 0x007ad87c));
    GP4_GLOBAL(uint32_t, 0x007a8e9e) = gp4::stdcall_<0x00504cf0, uint32_t>(GP4_GLOBAL(uint32_t, 0x007ad87c));

    GP4_GLOBAL(uint32_t, 0x00829930) = (GP4_GLOBAL(uint32_t, 0x007a8eaa) >> 3) *
                                       GP4_GLOBAL(uint32_t, 0x007a8ea2) *
                                       GP4_GLOBAL(uint32_t, 0x007a8ea6);
    uint32_t size;
    if (GP4_GLOBAL(uint8_t, 0x007ad869) != 0) {
        size = gp4::stdcall_<0x00504d10, uint32_t>(
            GP4_ARRAY(uint32_t, 0x007ad874)[GP4_GLOBAL(uint32_t, 0x007ad86c)]);
    } else {
        size = gp4::stdcall_<0x00504d10, uint32_t>(GP4_GLOBAL(uint32_t, 0x007ad870));
    }
    GP4_GLOBAL(uint32_t, 0x00829958) = size;

    GP4_GLOBAL(uint32_t, 0x007a8e72) = GP4_GLOBAL(uint32_t, 0x007a8ea2);
    GP4_GLOBAL(uint32_t, 0x007a8e7a) = (GP4_GLOBAL(uint32_t, 0x007a8eaa) >> 3) * GP4_GLOBAL(uint32_t, 0x007a8e72);
    GP4_GLOBAL(uint32_t, 0x007a8e24) =
        addrSpan(GP4_ARRAY(uint8_t, 0x00679b00), GP4_ARRAY(uint8_t, 0x0066d810)) + 0x2978u;

    uint32_t& block = GP4_GLOBAL(uint32_t, 0x008299a8);
    if (block == 1u) {
        gp4::cdecl_<0x005375f0, void>();
    } else {
        const uint32_t old = block;
        gp4::cdecl_<0x005375f0, void>();
        if (old != block) {
            gp4::cdecl_<0x00537508, void>();
        }
    }

    uint32_t rate;
    if (GP4_GLOBAL(uint8_t, 0x007ad869) != 0) {
        rate = GP4_GLOBAL(uint32_t, 0x007a8e46);
    } else {
        if (GP4_GLOBAL(uint8_t, 0x007ad816) != 0) {
            GP4_GLOBAL(uint32_t, 0x007a8e42) = 0xdd40a0u;
        } else {
            GP4_GLOBAL(uint32_t, 0x007a8e42) = 0xb71b00u;
        }
        rate = GP4_GLOBAL(uint32_t, 0x007a8e42);
    }
    GP4_GLOBAL(uint32_t, 0x008299d0) = rate;
    GP4_GLOBAL(uint32_t, 0x00829908) = GP4_GLOBAL(uint32_t, 0x006b35f0);

    const uint32_t* table = GP4_ARRAY(uint32_t, 0x00833ff0);
    const uint32_t first = table[GP4_GLOBAL(uint32_t, 0x007a8e9a)] + 2u;
    GP4_GLOBAL(uint32_t, 0x007a8e82) = first;
    GP4_GLOBAL(uint32_t, 0x00829980) = first * GP4_GLOBAL(uint32_t, 0x007a8e7a);
    const uint32_t second = table[GP4_GLOBAL(uint32_t, 0x007a8e9e)];
    GP4_GLOBAL(uint32_t, 0x007a8e92) = second + 2u;

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(second, 2u));
}
GP4_IMPL(0x00538a96, FUN_00538a96, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cb_00531648")
