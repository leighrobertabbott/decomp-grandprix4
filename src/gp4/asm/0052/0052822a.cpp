#include <gp4/gp4.h>
#include <gp4/flags.h>

// An entry of the 8-slot animation table at 0x0081cd91 (0x1c bytes, unaligned in the image).
#pragma pack(push, 1)
struct Unknown_AnimSlot {
    uint32_t State00;       // 0 free, 1 waiting, 2 running, 3 finished-looping
    uint32_t Desc04;        // Unknown_AnimDesc*
    uint32_t Countdown08;   // ticks until the next frame
    uint32_t Frame0C;       // frame number, the index the frame-cell closer 0x005280ac takes in eax
    uint32_t Loops10;
    uint32_t Wait14;
    uint32_t Flags18;       // bit 0: the frame advanced since the last pass
};

struct Unknown_AnimDesc {
    uint8_t Unknown00[0x04];
    uint32_t Unknown04;     // restart delay copied back to the slot's countdown
    uint8_t Unknown08[0x0c];
    uint32_t Flags14;       // 1, 4, 8, 0x10, 0x20 steer the placement offsets below
    uint32_t Unknown18;
    uint32_t Unknown1C;
    uint32_t Unknown20;
    uint32_t Unknown24;
    uint32_t Unknown28;
    uint32_t Unknown2C;
};
#pragma pack(pop)

// pushal wrapper over the animation slots in state 2 or 3: a slot that advanced since the last pass (flag bit 0)
// or every slot when al != 0 closes its frame cell (0x005280ac); when the cell is still live and 0x005388ba
// returns a nonzero pointer the descriptor values (+0x20..+0x2c) are queued into the sorted pool 0x00549db9,
// the frame record's fields are combined with the descriptor offsets and handed to 0x00531b3b. A slot with
// flag bit 0 then clears it and reloads its countdown from the descriptor (+4). popal restores every GPR; the
// flags are those of the final equal `cmp edx, 0x81ce71`.
// FUNCTION: GP4 0x0052822a
void __cdecl FUN_0052822a(gp4::Regs* r) {
    GP4_GLOBAL(uint8_t, 0x0081ce79) = (uint8_t)r->eax;

    gp4::Regs c = *r;
    Unknown_AnimSlot* const end = GP4_ARRAY(Unknown_AnimSlot, 0x0081ce71);
    for (Unknown_AnimSlot* slot = GP4_ARRAY(Unknown_AnimSlot, 0x0081cd91); slot != end; ++slot) {
        if (slot->State00 != 2u && slot->State00 != 3u) {
            continue;
        }
        c.edx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(slot));
        if ((slot->Flags18 & 1u) != 0u || GP4_GLOBAL(uint8_t, 0x0081ce79) != 0u) {
            c.eax = slot->Frame0C;
            gp4::call_regs(0x005280ac, c);
            if ((c.eflags & 1u) == 0u) {
                c.eax = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(c.edi));
                gp4::call_regs(0x005388ba, c);
                if (c.eax != 0u) {
                    const uint32_t pushed = c.eax;
                    const Unknown_AnimDesc* desc =
                        reinterpret_cast<const Unknown_AnimDesc*>(static_cast<uintptr_t>(slot->Desc04));
                    c.edi = slot->Desc04;
                    c.ebx = desc->Unknown20;
                    c.eax = desc->Unknown24;
                    c.ebp = desc->Unknown28;
                    c.ecx = desc->Unknown2C;
                    gp4::call_regs(0x00549db9, c);
                    if ((desc->Flags14 & 1u) != 0u) {
                        gp4::call_regs(0x0054a1d7, c);
                    }
                    const uint32_t* record = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(c.esi));
                    uint32_t x = record[0];
                    uint32_t y = record[1];
                    if ((desc->Flags14 & 4u) != 0u) {
                        x = desc->Unknown18;
                    } else if ((desc->Flags14 & 0x10u) != 0u) {
                        x += desc->Unknown18;
                    }
                    if ((desc->Flags14 & 8u) != 0u) {
                        y = desc->Unknown1C;
                    } else if ((desc->Flags14 & 0x20u) != 0u) {
                        y += desc->Unknown1C;
                    }
                    c.ebx = x;
                    c.eax = y;
                    c.ebp = record[2];
                    c.ecx = record[3];
                    c.esi = pushed;
                    gp4::call_regs(0x00531b3b, c);
                }
            }
        }
        if ((slot->Flags18 & 1u) != 0u) {
            slot->Flags18 &= 0xfffffffeu;
            const Unknown_AnimDesc* desc = reinterpret_cast<const Unknown_AnimDesc*>(static_cast<uintptr_t>(slot->Desc04));
            slot->Countdown08 = desc->Unknown04;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0x0081ce71u, 0x0081ce71u));
}
GP4_IMPL(0x0052822a, FUN_0052822a, "regs(eax:u32{=0,1,0x100,0xffffffff}) -> (cf, pf, af, zf, sf, of); globals=layout=anim_0052822a+slots_005388ed+cells_00527eae+slot8_005388ba+list_00549db9+pool_00549db9")
