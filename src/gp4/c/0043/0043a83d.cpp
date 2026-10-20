#include <gp4/gp4.h>

// A collision-object record: the position triple is all this routine reads directly.
struct Unknown_0043a83d_Object {
    int32_t X;
    int32_t Y;
    int32_t Z;
};

// Stores the 8-byte double at `from` into `to` (fld qword / fstp qword in the original).
static inline void copyDouble(uint32_t to, uint32_t from) {
    GP4_GLOBAL(double, to) = GP4_GLOBAL(double, from);
}

// FUNCTION: GP4 0x0043a83d
// Resolves a collision between the two objects [0x607abc] (esi) and [0x607ac0] (ebx). 0x0043a05a
// reserves a free slot of the 15-entry table 0x63db7c (CF clear when all are used: nothing to do).
// The vector between the two positions is turned into the rotation body 0x6231d8 (0x402305,
// 0x401d80, 0x401da2, 0x4020ef), 0x0043a785 distributes the response, and 0x0043a32a /
// 0x0043a81e record the contact in the object's slot table: first for ebx's side with
// [0x6097fc] = 0xff, then (a second slot) for the opposite side with [0x6097fc] = 0.
// Registers and flags are those the last called routine leaves.
void __cdecl FUN_0043a83d(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.esi = GP4_GLOBAL(uint32_t, 0x00607abc);
    c.ebx = GP4_GLOBAL(uint32_t, 0x00607ac0);
    const Unknown_0043a83d_Object* first = reinterpret_cast<const Unknown_0043a83d_Object*>(static_cast<uintptr_t>(c.esi));
    const Unknown_0043a83d_Object* second = reinterpret_cast<const Unknown_0043a83d_Object*>(static_cast<uintptr_t>(c.ebx));

    gp4::call_regs(0x0043a05a, c);
    if ((c.eflags & 1u) != 0u) {
        // slot index -> record pointer (0x34-byte shape records at 0x63db8c)
        GP4_GLOBAL(uint32_t, 0x00609828) = c.eax;
        c.eax = c.eax * 0x34u + 0x0063db8cu;
        GP4_GLOBAL(uint32_t, 0x0060982c) = c.eax;

        const int32_t dx = second->X - first->X;
        GP4_GLOBAL(int32_t, 0x006f2bf4) = dx;
        GP4_GLOBAL(double, 0x00622c80) = static_cast<double>(GP4_GLOBAL(int32_t, 0x006f2bf4));
        const int32_t dy = second->Y - first->Y;
        GP4_GLOBAL(int32_t, 0x006f2bf4) = dy;
        GP4_GLOBAL(double, 0x00622c88) = static_cast<double>(GP4_GLOBAL(int32_t, 0x006f2bf4));
        const int32_t dz = second->Z - first->Z;
        GP4_GLOBAL(int32_t, 0x006f2bf4) = dz;
        GP4_GLOBAL(double, 0x00622c90) = static_cast<double>(GP4_GLOBAL(int32_t, 0x006f2bf4));
        c.eax = static_cast<uint32_t>(dz);

        gp4::call_regs(0x00402305, c);

        c.edx = 0x006231d8u;
        copyDouble(0x006231e8, 0x00622c98);
        copyDouble(0x006231f0, 0x00622ca0);
        copyDouble(0x006231f8, 0x00622ca8);
        gp4::call_regs(0x00401d80, c);
        gp4::call_regs(0x00401da2, c);

        c.eax = 0x006231d8u;
        c.edx = 0x00623038u;
        c.ecx = 0x006232a8u;
        gp4::call_regs(0x004020ef, c);

        c.eax = c.ebx;
        c.edx = c.esi;
        gp4::call_regs(0x0043a785, c);

        GP4_GLOBAL(uint8_t, 0x006097fc) = 0xff;
        gp4::call_regs(0x0043a32a, c);
        if ((c.eflags & 1u) == 0u) {
            c.eax = GP4_GLOBAL(uint32_t, 0x00609828);
            const uint32_t savedEsi = c.esi;
            c.esi = c.ebx;
            gp4::call_regs(0x0043a81e, c);
            c.esi = savedEsi;
        }

        gp4::call_regs(0x0043a05a, c);
        if ((c.eflags & 1u) != 0u) {
            GP4_GLOBAL(uint32_t, 0x00609828) = c.eax;
            c.eax = c.eax * 0x34u + 0x0063db8cu;
            GP4_GLOBAL(uint32_t, 0x0060982c) = c.eax;

            c.eax = 0x006231d8u;
            c.edx = 0x00623108u;
            c.ecx = 0x006232a8u;
            gp4::call_regs(0x004020ef, c);

            c.eax = c.esi;
            c.edx = c.ebx;
            gp4::call_regs(0x0043a785, c);

            GP4_GLOBAL(uint8_t, 0x006097fc) = 0;
            const uint32_t savedEsi = c.esi;
            c.esi = c.ebx;
            gp4::call_regs(0x0043a32a, c);
            c.esi = savedEsi;
            if ((c.eflags & 1u) == 0u) {
                c.eax = GP4_GLOBAL(uint32_t, 0x00609828);
                gp4::call_regs(0x0043a81e, c);
            }
        }
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x0043a83d, FUN_0043a83d, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); cases=300; globals=fuzz:i32+layout=state_0043acb6+slots_0043a05a+carcc_0043a785+shape_006099c0+bounds_0043a295+view_00609860+ang_006231d8+objs_0043a83d+objs2_0043a83d")
