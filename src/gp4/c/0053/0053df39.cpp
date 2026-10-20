#include <gp4/gp4.h>
#include <gp4/flags.h>

// A menu node: next pointer first, a value-type byte (low 2 bits: 0 byte, 1 word, 2 dword, 3 string)
// at +0xd, a destination pointer at +0xe and a flag byte at +0x1e (bit 0 = restore this value).
// Packed: the pointer at +0x0e and the flag byte at +0x1e are not naturally aligned.
#pragma pack(push, 1)
struct Unknown_MenuNode {
    uint32_t Next;                  // +0x00
    uint8_t Unknown04[9];           // +0x04
    uint8_t ValueType;              // +0x0d
    uint32_t Destination;           // +0x0e
    uint8_t Unknown12[0x0c];        // +0x12
    uint8_t Flags1E;                // +0x1e
};
#pragma pack(pop)
static_assert(sizeof(Unknown_MenuNode) == 0x1f, "Unknown_MenuNode packed size");

static inline uint8_t* bytesAt(uint32_t address) {
    return reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(address));
}

// FUNCTION: GP4 0x0053df39
// Restores menu values from a saved stream: first the blocks listed at [[0x0082fd34] + 0x28]
// (6-byte entries: word count, dword destination; copied byte by byte from the stream pointer
// [0x0082fd38] until a zero count), then for every node of the chain [0x0082fd34] that has bit 0 of
// +0x1e set the value of its +0xd type (byte, word, dword, or a NUL terminated string) is copied
// from the stream to the node's +0xe pointer. Stops when the stream pointer reaches [0x0082fd4e]
// or the chain ends. The stream pointer is kept in esi (the global is not updated). The original's
// jump table has a fifth, unreachable slot (type is masked with 3) that calls 0x54e232; omitted.
void __cdecl FUN_0053df39(gp4::Regs* r) {
    uint32_t eax = r->eax;
    uint32_t ecx = r->ecx;
    uint32_t edi = r->edi;
    uint32_t esi = GP4_GLOBAL(uint32_t, 0x0082fd38);
    uint32_t block = GP4_GLOBAL(uint32_t, 0x0082fd34) + 0x28u;

    while (*reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(block)) != 0u) {
        ecx = *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(block));
        edi = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(block + 2u));
        while (ecx != 0u) {
            *bytesAt(edi) = *bytesAt(esi);
            esi += 1u;
            edi += 1u;
            ecx -= 1u;
        }
        block += 6u;
    }

    uint32_t node = GP4_GLOBAL(uint32_t, 0x0082fd34);
    uint32_t flags;
    for (;;) {
        flags = gp4::flags::sub32(esi, GP4_GLOBAL(uint32_t, 0x0082fd4e));
        if (!(esi < GP4_GLOBAL(uint32_t, 0x0082fd4e))) {
            break;
        }
        node = reinterpret_cast<const Unknown_MenuNode*>(static_cast<uintptr_t>(node))->Next;
        flags = gp4::flags::logic32(node);
        if (node == 0u) {
            break;
        }
        const Unknown_MenuNode* item = reinterpret_cast<const Unknown_MenuNode*>(static_cast<uintptr_t>(node));
        if ((item->Flags1E & 1u) == 0u) {
            continue;
        }
        edi = item->Destination;
        const uint32_t type = item->ValueType & 3u;
        eax = 0x0053df94u + type * 4u;
        switch (type) {
        case 0:
            *bytesAt(edi) = *bytesAt(esi);
            esi += 1u;
            edi += 1u;
            break;
        case 1:
            *bytesAt(edi) = *bytesAt(esi);
            *bytesAt(edi + 1u) = *bytesAt(esi + 1u);
            esi += 2u;
            edi += 2u;
            break;
        case 2:
            *bytesAt(edi) = *bytesAt(esi);
            *bytesAt(edi + 1u) = *bytesAt(esi + 1u);
            *bytesAt(edi + 2u) = *bytesAt(esi + 2u);
            *bytesAt(edi + 3u) = *bytesAt(esi + 3u);
            esi += 4u;
            edi += 4u;
            break;
        default: {
            uint32_t c;
            do {
                c = *bytesAt(esi);
                esi += 1u;
                *bytesAt(edi) = static_cast<uint8_t>(c);
                edi += 1u;
            } while (c != 0u);
            eax = (eax & 0xffffff00u) | c;
            break;
        }
        }
    }

    r->eax = eax;
    r->ebx = node;
    r->ecx = ecx;
    r->esi = esi;
    r->edi = edi;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0053df39, FUN_0053df39, "regs(eax:u32, ecx:u32, edi:u32) -> (eax, ebx, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=menu_0084300a+mitems_0053ddbb+restore_0053df39")
