#include <gp4/gp4.h>
#include <gp4/flags.h>

// Snapshots the selected car record (esi = [0x00679d30], edi = its dword +0x1c) into the HUD globals:
// 0x00679b6c = (|[+0x98]| >> 8) >> 8 via 0x0040824b, the qword [+0] via 0x004045d3 into
// 0x00934df0/0x00934df4, and the triple of 0x00402f8a (eax = sx16 [+0x12], edx = [+8], ecx = sx16 [+0x16])
// into 0x00934de8, 0x00934df8 and the word 0x00934e88. The three dwords are then stored >> 5 (sar) in
// 0x00679c0c, 0x00679c10 and 0x00679c14, and the words [+0xe], 0x00934de8 and 0x00934e88 in
// 0x00679c18/1a/1c. eax ends with its upper half from the last sar and its low word from cx; the flags
// are those of the last `sar eax, 5`.
// FUNCTION: GP4 0x00526465
void __cdecl FUN_00526465(gp4::Regs* r) {
    const uint32_t record = GP4_GLOBAL(uint32_t, 0x00679d30);
    const uint32_t edi = GP4_FIELD(uint32_t, record, 0x1c);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.esi = record;
    c.edi = edi;

    gp4::call_regs(0x0040824b, c);
    GP4_GLOBAL(int32_t, 0x00679b6c) = static_cast<int32_t>(c.eax) >> 8;

    gp4::call_regs(0x004045d3, c);
    GP4_GLOBAL(uint32_t, 0x00934df0) = c.eax;
    GP4_GLOBAL(uint32_t, 0x00934df4) = c.edx;

    gp4::call_regs(0x00402f8a, c);
    GP4_GLOBAL(uint32_t, 0x00934de8) = c.eax;
    GP4_GLOBAL(uint32_t, 0x00934df8) = c.edx;
    GP4_GLOBAL(uint16_t, 0x00934e88) = static_cast<uint16_t>(c.ecx);

    GP4_GLOBAL(int32_t, 0x00679c0c) = GP4_GLOBAL(int32_t, 0x00934df0) >> 5;
    GP4_GLOBAL(int32_t, 0x00679c10) = GP4_GLOBAL(int32_t, 0x00934df8) >> 5;
    const int32_t last = GP4_GLOBAL(int32_t, 0x00934df4) >> 5;
    GP4_GLOBAL(int32_t, 0x00679c14) = last;

    GP4_GLOBAL(uint16_t, 0x00679c18) = GP4_FIELD(uint16_t, record, 0x0e);
    GP4_GLOBAL(uint16_t, 0x00679c1a) = GP4_GLOBAL(uint16_t, 0x00934de8);
    const uint16_t tail = GP4_GLOBAL(uint16_t, 0x00934e88);
    GP4_GLOBAL(uint16_t, 0x00679c1c) = tail;

    // Flags of the last `sar eax, 5`: CF = last bit shifted out, OF = 0, AF = 0, ZF/SF/PF from the result.
    const uint32_t original = static_cast<uint32_t>(GP4_GLOBAL(int32_t, 0x00934df4));
    const uint32_t flags = gp4::flags::logic32(static_cast<uint32_t>(last)) | ((original >> 4) & 1u);

    r->eax = (static_cast<uint32_t>(last) & 0xffff0000u) | tail;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = record;
    r->edi = edi;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00526465, FUN_00526465, "regs() -> (eax, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ptr_00679d30")
