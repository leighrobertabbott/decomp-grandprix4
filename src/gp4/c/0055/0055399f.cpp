#include <gp4/gp4.h>
#include <gp4/flags.h>

// 13 entries of 0xe0 bytes at 0x866248: the signed time at +0x6c is raised to
// the clock [0x66d814] when it is behind, and the result is copied to +0xd8 and
// +0xdc. Leaves eax = last value, ebx = end of the table, ecx = 0 and the flags
// of the final `add ebx, 0xe0` (loop does not touch them).

struct Unknown_866248 {
    uint8_t Unknown00[0x6c];
    int32_t Time6c;
    uint8_t Unknown70[0xd8 - 0x70];
    int32_t TimeD8;
    int32_t TimeDC;
};
static_assert(sizeof(Unknown_866248) == 0xe0, "entry stride");

// FUNCTION: GP4 0x0055399f
void __cdecl FUN_0055399f(gp4::Regs* r) {
    Unknown_866248* entry = reinterpret_cast<Unknown_866248*>(static_cast<uintptr_t>(0x00866248));
    uint32_t value = 0;
    uint32_t flags = 0;
    for (int left = 13; left != 0; --left) {
        value = static_cast<uint32_t>(entry->Time6c);
        if (static_cast<int32_t>(value) < GP4_GLOBAL(int32_t, 0x0066d814)) {
            value = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x0066d814));
        }
        entry->Time6c = static_cast<int32_t>(value);
        entry->TimeD8 = static_cast<int32_t>(value);
        entry->TimeDC = static_cast<int32_t>(value);
        flags = gp4::flags::add32(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(entry)), 0xe0u);
        ++entry;
    }
    r->eax = value;
    r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(entry));
    r->ecx = 0;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0055399f, FUN_0055399f, "regs() -> (eax, ebx, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
