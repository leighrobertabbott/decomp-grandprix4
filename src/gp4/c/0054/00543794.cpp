#include <gp4/gp4.h>
#include <gp4/flags.h>

// Calls 0x531af4 (stores the low word of eax at 0x827559) with eax = value.
static __declspec(noinline) void StoreWord(const gp4::Regs* context, uint32_t value) {
    gp4::Regs c = *context;
    c.eax = value;
    gp4::call_regs(0x00531af4, c);
}

// FUNCTION: GP4 0x00543794
// Registers a rectangle record (esi: words +4 x0, +6 key, +8 x1, +0xa y1, byte +0xc mode) in the
// sorted pool list. First 0x54cc96 stores the colour 0x7fff, then 0x549db9 inserts
// {x0, key, x1, y1} (ebx, eax, ebp, ecx). A tag word is then written through 0x531af4: 0x4210 when
// the record's mode byte equals [0x82fd45], else 0x2108 followed by 0x2529. Leaves eax = key,
// ebx = x0, ebp = x1, ecx = y1, dl = [0x82fd45] (callers push/pop eax); the flags are those of the
// last `and eax, 0xffff` in 0x531af4 (the empty routines 0x531b2b/0x531b2e keep them).
void __cdecl FUN_00543794(gp4::Regs* r) {
    const uint32_t record = r->esi;

    gp4::Regs colour = *r;
    colour.eax = 0x7fffu;
    gp4::call_regs(0x0054cc96, colour);

    const uint32_t x0 = GP4_FIELD(uint16_t, record, 4);
    const uint32_t key = GP4_FIELD(uint16_t, record, 6);
    const uint32_t x1 = GP4_FIELD(uint16_t, record, 8);
    const uint32_t y1 = GP4_FIELD(uint16_t, record, 0xa);

    gp4::Regs insert = colour;
    insert.ebx = x0;
    insert.eax = key;
    insert.ebp = x1;
    insert.ecx = y1;
    gp4::call_regs(0x00549db9, insert);

    const uint32_t mode = GP4_GLOBAL(uint8_t, 0x0082fd45);
    uint32_t tag;
    if (mode == GP4_FIELD(uint8_t, record, 0xc)) {
        StoreWord(&insert, 0x4210u);
        gp4::cdecl_<0x00531b2e, void>();
        tag = 0x4210u;
    } else {
        StoreWord(&insert, 0x2108u);
        gp4::cdecl_<0x00531b2e, void>();
        StoreWord(&insert, 0x2529u);
        gp4::cdecl_<0x00531b2b, void>();
        tag = 0x2529u;
    }

    r->eax = key;
    r->ebx = x0;
    r->ebp = x1;
    r->ecx = y1;
    r->edx = (r->edx & 0xffffff00u) | mode;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(tag));
}
GP4_IMPL(0x00543794, FUN_00543794, "regs(ebx:u32, ecx:u32, edx:u32, esi:ptr[0x10]:u8{=0,1,0xff}) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9")
