#include <gp4/gp4.h>
#include <gp4/flags.h>

#pragma pack(push, 1)
// A level of the chain hanging off the header at [0x0082fd34] (the header itself is level 0).
struct Level_0054eb99 {
    Level_0054eb99* Parent;     // +0x00
    int16_t Unknown04;          // +0x04
    int16_t Unknown06;          // +0x06
    int16_t Unknown08;          // +0x08
    int16_t Unknown0A;          // +0x0a
    uint8_t Unknown0C[6];       // +0x0c
    uint32_t Unknown12;         // +0x12 index into the 0x00835846 dword table (0x0054d1ad)
    uint8_t Unknown16[6];       // +0x16
    void* Unknown1C;            // +0x1c (header only: first item node)
    uint8_t Unknown20[0x0e];    // +0x20
    int16_t Unknown2E;          // +0x2e
};
struct Node_0054eb99 {
    Node_0054eb99* Next;        // +0x00
    int16_t Unknown04;          // +0x04
    int16_t Unknown06;          // +0x06
    int16_t Unknown08;          // +0x08
    int16_t Unknown0A;          // +0x0a
    uint8_t Unknown0C[4];       // +0x0c
    uint8_t Flags10;            // +0x10
    uint8_t Unknown11;          // +0x11
    uint8_t Depth12;            // +0x12 number of parent links to follow from the header
};
#pragma pack(pop)

// The +0x04 adjustment for a flagged node: word +4 of the level minus (8 + k), where k (0 or 1) is the eax
// the original loads before calling 0x0054cb71 (preserves eax), 0x0054d1ad (esi only) and 0x0054d6f2 (ret).
static inline uint16_t adjust_0054eb99(const gp4::Regs* r, Level_0054eb99* level, uint32_t k) {
    gp4::Regs c = *r;
    c.eax = k;
    c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(level));
    gp4::call_regs(0x0054cb71, c);
    c.esi = level->Unknown12;
    gp4::call_regs(0x0054d1ad, c);
    gp4::cdecl_<0x0054d6f2, void>();
    return static_cast<uint16_t>(static_cast<uint16_t>(level->Unknown04) - static_cast<uint16_t>(c.eax + 8u));
}

// Walks the item list of the header [0x0082fd34] (+0x1c, next at +0): every node with flag 0x80 is offset
// by the level it names (+0x12 = depth along the header's parent chain) and the flag is cleared. Exit:
// eax = 0, edi = 0, esi = the last level used (entry esi if none), flags of `or edi, edi`.
// FUNCTION: GP4 0x0054eb99
void __cdecl FUN_0054eb99(gp4::Regs* r) {
    Level_0054eb99* const header = GP4_GLOBAL(Level_0054eb99*, 0x0082fd34);
    uint32_t esi = r->esi;
    for (Node_0054eb99* node = static_cast<Node_0054eb99*>(header->Unknown1C); node != nullptr; node = node->Next) {
        if ((node->Flags10 & 0x80u) == 0u) {
            continue;
        }
        Level_0054eb99* level = GP4_GLOBAL(Level_0054eb99*, 0x0082fd34);
        for (uint32_t depth = node->Depth12; depth != 0u; --depth) {
            level = level->Parent;
        }
        esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(level));
        uint16_t dx;
        if ((node->Flags10 & 0x20u) != 0u) {
            dx = adjust_0054eb99(r, level, 0u);
        } else if ((node->Flags10 & 0x40u) != 0u) {
            dx = adjust_0054eb99(r, level, 1u);
        } else {
            dx = static_cast<uint16_t>(level->Unknown04);
        }
        node->Unknown04 = static_cast<int16_t>(static_cast<uint16_t>(node->Unknown04) + dx);
        node->Unknown06 = static_cast<int16_t>(static_cast<uint16_t>(node->Unknown06) + static_cast<uint16_t>(level->Unknown06));
        const uint16_t dy = static_cast<uint16_t>((node->Flags10 & 0x10u) != 0u ? level->Unknown2E : level->Unknown08);
        node->Unknown08 = static_cast<int16_t>(static_cast<uint16_t>(node->Unknown08) + dy);
        node->Unknown0A = static_cast<int16_t>(static_cast<uint16_t>(node->Unknown0A) + static_cast<uint16_t>(level->Unknown0A));
        node->Flags10 = static_cast<uint8_t>(node->Flags10 & 0x7fu);
    }
    gp4::Regs c = *r;
    c.eax = 0u;
    c.esi = esi;
    gp4::call_regs(0x0054cb71, c);
    GP4_GLOBAL(uint8_t, 0x00838e3e) = 0u;
    r->eax = 0u;
    r->esi = esi;
    r->edi = 0u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(0u));
}
GP4_IMPL(0x0054eb99, FUN_0054eb99, "regs(esi:u32) -> (eax, esi, edi, cf, pf, af, zf, sf, of); globals=layout=hdr_0054eb99")
