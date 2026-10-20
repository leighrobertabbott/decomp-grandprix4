#include <gp4/gp4.h>
#include <gp4/flags.h>

#pragma pack(push, 1)
// Hotspot node of the current menu (the 0x0054ede8 list): rectangle words and a text index.
struct Hotspot_54ede8 {
    uint32_t Next;          // +0x00
    uint16_t X0;            // +0x04
    uint16_t Y0;            // +0x06
    uint16_t X1;            // +0x08
    uint16_t Y1;            // +0x0a
    uint32_t TextIndex;     // +0x0c index into the dword table 0x00835846 (0x0054d1ad)
};
#pragma pack(pop)

// Places the box for the hovered hotspot [0x00838e48] (no-op when null): settles a pending amount
// (0x0054ec64), looks up the node's text (0x0054d1ad; the measure call 0x0054d6f4 is a stub, so the
// box size stays ebx/[0x00838e70]), sets the box origin [0x00838e60] = ebx + 0x10 and
// [0x00838e64] = [0x00838e70] + 0xc, picks the y offset above or below the cursor y [0x00838e44]
// by the node's vertical midline, and clamps the far corner [0x00838e58]/[0x00838e5c] (from the
// cursor x [0x00838e40], at least 8) to the screen size [0x00822244]/[0x00822248] - 8, storing
// the extents [0x00838e50]/[0x00838e54]. The flags are those of `cmp [0x838e70], 1`.
// FUNCTION: GP4 0x0054ee4f
void __cdecl FUN_0054ee4f(gp4::Regs* r) {
    if (GP4_GLOBAL(uint32_t, 0x00838e48) == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0u, 0u));
        return;
    }
    if (GP4_GLOBAL(uint32_t, 0x00838e88) != 0u) {
        gp4::call_regs(0x0054ec64, *r);
    }
    r->eax = GP4_GLOBAL(uint32_t, 0x00838e70);
    gp4::call_regs(0x0054cb71, *r);
    r->esi = reinterpret_cast<const Hotspot_54ede8*>(GP4_GLOBAL(uint32_t, 0x00838e48))->TextIndex;
    gp4::call_regs(0x0054d1ad, *r);
    gp4::call_regs(0x0054d6f4, *r);

    const uint32_t left = r->ebx + 0x10u;
    const uint32_t top = r->eax + 0xcu;
    GP4_GLOBAL(uint32_t, 0x00838e60) = left;
    GP4_GLOBAL(uint32_t, 0x00838e64) = top;

    const Hotspot_54ede8* node = reinterpret_cast<const Hotspot_54ede8*>(GP4_GLOBAL(uint32_t, 0x00838e48));
    r->esi = reinterpret_cast<uint32_t>(node);
    const uint32_t mid = static_cast<uint32_t>(static_cast<uint16_t>(node->Y0 + node->Y1)) >> 1;
    const uint32_t cursor_y = GP4_GLOBAL(uint32_t, 0x00838e44);
    uint32_t dy;
    if (mid > cursor_y) {
        dy = cursor_y - top;
    } else {
        dy = cursor_y + 0xcu;
    }

    uint32_t dx = GP4_GLOBAL(uint32_t, 0x00838e40);
    if (static_cast<int32_t>(dx) < 8) {
        dx = 8u;
    }
    uint32_t right = dx + left;
    const uint32_t max_x = GP4_GLOBAL(uint32_t, 0x00822244) - 8u;
    if (!(static_cast<int32_t>(right) < static_cast<int32_t>(max_x))) {
        right = max_x;
    }
    GP4_GLOBAL(uint32_t, 0x00838e58) = right;
    const uint32_t width = right - left + 1u;
    GP4_GLOBAL(uint32_t, 0x00838e50) = width;
    r->ebx = width;

    if (static_cast<int32_t>(dy) < 8) {
        dy = 8u;
    }
    const uint32_t max_y = GP4_GLOBAL(uint32_t, 0x00822248) - 8u;
    r->edx = max_y;
    uint32_t bottom = dy + top;
    if (!(static_cast<int32_t>(bottom) < static_cast<int32_t>(max_y))) {
        bottom = max_y;
    }
    GP4_GLOBAL(uint32_t, 0x00838e5c) = bottom;
    const uint32_t height = bottom - top + 1u;
    GP4_GLOBAL(uint32_t, 0x00838e54) = height;
    r->eax = height;

    const uint32_t mode = GP4_GLOBAL(uint32_t, 0x00838e70);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(mode, 1u));
    if (mode != 1u) {
        r->eax = 1u;
        gp4::call_regs(0x0054cb71, *r);
    }
}
GP4_IMPL(0x0054ee4f, FUN_0054ee4f, "regs(ebx:u32) -> (eax, ebx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=tip_0054ee4f+pool_00538a24")
