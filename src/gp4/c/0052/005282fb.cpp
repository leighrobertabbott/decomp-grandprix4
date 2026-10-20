#include <gp4/gp4.h>
#include <gp4/flags.h>

// An entry of the 8-slot animation table at 0x0081cd91 (0x1c bytes, unaligned in the image).
#pragma pack(push, 1)
struct Unknown_AnimSlot {
    uint32_t State00;       // 0 free, 1 waiting, 2 running, 3 finished-looping
    uint32_t Desc04;        // Unknown_AnimDesc*
    uint32_t Countdown08;   // ticks until the next frame (runs below zero)
    uint32_t Frame0C;
    uint32_t Loops10;
    uint32_t Wait14;        // dword* that must be 0 for a waiting slot to start
    uint32_t Flags18;
};

struct Unknown_AnimDesc {
    uint8_t Unknown00[0x0c];
    uint32_t FirstFrame0C;
    uint32_t LastFrame10;
    uint32_t Flags14;
};
#pragma pack(pop)

// FUNCTION: GP4 0x005282fb
// pushal wrapper: advances the 8 animation slots. A running slot (state 2) counts its delay down; when it runs
// below zero it sets bit 0 of its flags, steps its frame, and past the descriptor's last frame wraps to the first
// one and, when a loop count is set, counts it down: at zero the slot becomes 3 (descriptor flag 0x100) or 0 (free).
// A second pass starts the waiting slots (state 1) whose wait cell is zero. popal restores every GPR; the flags
// are those of the final equal `cmp edx, 0x81ce71`.
void __cdecl FUN_005282fb(gp4::Regs* r) {
    Unknown_AnimSlot* const end = GP4_ARRAY(Unknown_AnimSlot, 0x0081ce71);
    for (Unknown_AnimSlot* slot = GP4_ARRAY(Unknown_AnimSlot, 0x0081cd91); slot != end; ++slot) {
        if (slot->State00 != 2u) {
            continue;
        }
        slot->Countdown08 -= 1u;
        if ((slot->Countdown08 & 0x80000000u) == 0u) {
            continue;
        }
        slot->Flags18 |= 1u;
        const Unknown_AnimDesc* desc = reinterpret_cast<const Unknown_AnimDesc*>(static_cast<uintptr_t>(slot->Desc04));
        slot->Frame0C += 1u;
        if (slot->Frame0C <= desc->LastFrame10) {
            continue;
        }
        slot->Frame0C = desc->FirstFrame0C;
        if (slot->Loops10 == 0u) {
            continue;
        }
        slot->Loops10 -= 1u;
        if (slot->Loops10 != 0u) {
            continue;
        }
        slot->State00 = (desc->Flags14 & 0x100u) != 0u ? 3u : 0u;
    }
    for (Unknown_AnimSlot* slot = GP4_ARRAY(Unknown_AnimSlot, 0x0081cd91); slot != end; ++slot) {
        if (slot->State00 == 1u && *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(slot->Wait14)) == 0u) {
            slot->State00 = 2u;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0x0081ce71u, 0x0081ce71u));
}
GP4_IMPL(0x005282fb, FUN_005282fb, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=anim_005282fb")
