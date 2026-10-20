#include <gp4/gp4.h>
#include <gp4/flags.h>

// Run list node (0x14 bytes, the 0x00831d81 list of 0x0054a057/0x00549db9): +0 next, +4 and +0xc
// keys, +8 start, +0x10 end.
#pragma pack(push, 1)
struct Unknown_00549f22_Run {
    uint32_t Next00;
    int32_t Unknown04;
    int32_t Start08;
    int32_t Unknown0C;
    int32_t End10;
};
#pragma pack(pop)

static inline Unknown_00549f22_Run* run_at(uint32_t p) {
    return reinterpret_cast<Unknown_00549f22_Run*>(static_cast<uintptr_t>(p));
}

// Splits overlapping neighbours of the run list whose head cell is the dword 0x00831d81 (ebx walks
// the cells, ebx = [ebx]). For a pair a = [ebx], b = [a] with a's end >= b's start (unsigned) a node
// c is popped from the free list [0x00831d85] (empty: the 0x0054e232 error stub with esi = 0x00549f55,
// then return) and linked between them: c covers the overlap (start b+8, the smaller +4, the larger
// +0xc), a keeps the part before it and b the part after; nodes left with end < start are unlinked
// onto the free list, then the bubble pass 0x0054a09f re-sorts from the cell. The walk ends at a null
// node; every exit leaves the flags of `or reg, reg` on a zero register (ZF|PF).
// FUNCTION: GP4 0x00549f22
void __cdecl FUN_00549f22(gp4::Regs* r) {
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t edi = r->edi;
    uint32_t ebx = 0x00831d81u;
    uint32_t esi;

    for (;;) {
        esi = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(ebx));
        if (esi == 0u) break;
        edi = run_at(esi)->Next00;
        if (edi == 0u) break;
        Unknown_00549f22_Run* a = run_at(esi);
        Unknown_00549f22_Run* b = run_at(edi);
        eax = static_cast<uint32_t>(a->End10);
        if (eax >= static_cast<uint32_t>(b->Start08)) {
            edx = GP4_GLOBAL(uint32_t, 0x00831d85);
            if (edx == 0u) {
                gp4::Regs e = *r;
                e.eax = eax;
                e.ebx = ebx;
                e.edx = edx;
                e.esi = 0x00549f55u;
                e.edi = edi;
                gp4::call_regs(0x0054e232, e);
                break;
            }
            Unknown_00549f22_Run* c = run_at(edx);
            GP4_GLOBAL(uint32_t, 0x00831d85) = c->Next00;
            a->Next00 = edx;
            c->Next00 = edi;
            c->Unknown04 = a->Unknown04 < b->Unknown04 ? a->Unknown04 : b->Unknown04;
            c->Unknown0C = a->Unknown0C > b->Unknown0C ? a->Unknown0C : b->Unknown0C;
            c->Start08 = b->Start08;
            if (a->End10 <= b->End10) {
                const int32_t end = a->End10;
                c->End10 = end;
                a->End10 = b->Start08 - 1;
                b->Start08 = end + 1;
            } else {
                c->End10 = b->End10;
                const int32_t start = b->Start08;
                b->Unknown04 = a->Unknown04;
                b->Start08 = b->End10 + 1;
                b->Unknown0C = a->Unknown0C;
                b->End10 = a->End10;
                a->End10 = start - 1;
            }
            eax = static_cast<uint32_t>(a->End10);
            if (a->End10 < a->Start08) {
                *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(ebx)) = edx;
                a->Next00 = GP4_GLOBAL(uint32_t, 0x00831d85);
                GP4_GLOBAL(uint32_t, 0x00831d85) = esi;
                eax = a->Next00;
            }
            eax = static_cast<uint32_t>(c->End10);
            if (c->End10 < c->Start08) {
                a->Next00 = edi;
                c->Next00 = GP4_GLOBAL(uint32_t, 0x00831d85);
                GP4_GLOBAL(uint32_t, 0x00831d85) = edx;
                eax = c->Next00;
            }
            eax = static_cast<uint32_t>(b->End10);
            if (b->End10 < b->Start08) {
                c->Next00 = b->Next00;
                b->Next00 = GP4_GLOBAL(uint32_t, 0x00831d85);
                GP4_GLOBAL(uint32_t, 0x00831d85) = edi;
                eax = b->Next00;
            }
            gp4::Regs s = *r;
            s.eax = eax;
            s.ebx = ebx;
            s.edx = edx;
            s.esi = esi;
            s.edi = edi;
            gp4::call_regs(0x0054a09f, s);
        }
        ebx = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(ebx));
    }

    r->eax = eax;
    r->ebx = ebx;
    r->edx = edx;
    r->esi = esi;
    r->edi = edi;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(0u));
}
GP4_IMPL(0x00549f22, FUN_00549f22, "regs(eax:u32, edx:u32, edi:u32) -> (eax, ebx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=split_00549f22")
