#include <gp4/gp4.h>
#include <gp4/flags.h>

// Bounding box of a frame range: edi addresses an object whose dwords +0xc/+0x10 are the first and
// last index into the 0x18-byte records at 0x8174f8 (dwords +0 x, +4 y, +8 width, +0xc height).
// Flags dword +0x14: bit 2 / bit 3 replace x / y by +0x18 / +0x1c, else bit 4 / bit 5 add them.
// The running box lives in 0x81cd78 (min x, from 800), 0x81cd7c (min y, from 600), 0x81cd80
// (max x) and 0x81cd84 (max y), and is copied to +0x20/+0x24/+0x28/+0x2c. eax ends as max y,
// ebx one record past the last, ecx = 0, flags from the final `dec ecx` (CF of `add ebx, 0x18`).
// FUNCTION: GP4 0x00528379
void __cdecl FUN_00528379(gp4::Regs* r) {
    uint8_t* const obj = (uint8_t*)(uintptr_t)r->edi;
    const uint32_t first = *(const uint32_t*)(obj + 0x0c);
    const uint32_t last = *(const uint32_t*)(obj + 0x10);
    uint32_t count = last - first + 1u;
    uint32_t rec = first * 0x18u + 0x008174f8u;
    uint32_t flags;

    GP4_GLOBAL(int32_t, 0x0081cd78) = 800;
    GP4_GLOBAL(int32_t, 0x0081cd80) = 0;
    GP4_GLOBAL(int32_t, 0x0081cd7c) = 600;
    GP4_GLOBAL(int32_t, 0x0081cd84) = 0;

    do {
        const uint32_t* const frame = (const uint32_t*)(uintptr_t)rec;
        uint32_t v = frame[0];
        if ((*(const uint32_t*)(obj + 0x14) & 4u) != 0u) {
            v = *(const uint32_t*)(obj + 0x18);
        } else if ((*(const uint32_t*)(obj + 0x14) & 0x10u) != 0u) {
            v += *(const uint32_t*)(obj + 0x18);
        }
        if ((int32_t)v < GP4_GLOBAL(int32_t, 0x0081cd78)) {
            GP4_GLOBAL(int32_t, 0x0081cd78) = (int32_t)v;
        }
        v = v + frame[2] - 1u;
        if ((int32_t)v > GP4_GLOBAL(int32_t, 0x0081cd80)) {
            GP4_GLOBAL(int32_t, 0x0081cd80) = (int32_t)v;
        }

        v = frame[1];
        if ((*(const uint32_t*)(obj + 0x14) & 8u) != 0u) {
            v = *(const uint32_t*)(obj + 0x1c);
        } else if ((*(const uint32_t*)(obj + 0x14) & 0x20u) != 0u) {
            v += *(const uint32_t*)(obj + 0x1c);
        }
        if ((int32_t)v < GP4_GLOBAL(int32_t, 0x0081cd7c)) {
            GP4_GLOBAL(int32_t, 0x0081cd7c) = (int32_t)v;
        }
        v = v + frame[3] - 1u;
        if ((int32_t)v > GP4_GLOBAL(int32_t, 0x0081cd84)) {
            GP4_GLOBAL(int32_t, 0x0081cd84) = (int32_t)v;
        }

        flags = gp4::flags::add32(rec, 0x18u);
        rec += 0x18u;
        flags = gp4::flags::dec32(count, flags);
        --count;
    } while (count != 0u);

    *(int32_t*)(obj + 0x20) = GP4_GLOBAL(int32_t, 0x0081cd78);
    *(int32_t*)(obj + 0x24) = GP4_GLOBAL(int32_t, 0x0081cd7c);
    *(int32_t*)(obj + 0x28) = GP4_GLOBAL(int32_t, 0x0081cd80);
    *(int32_t*)(obj + 0x2c) = GP4_GLOBAL(int32_t, 0x0081cd84);

    r->eax = (uint32_t)GP4_GLOBAL(int32_t, 0x0081cd84);
    r->ebx = rec;
    r->ecx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00528379, FUN_00528379, "regs(edi:ptr[0x38]:layout=obj_00528379) -> (eax, ebx, ecx, cf, pf, af, zf, sf, of); globals=layout=frames_008174f8")
