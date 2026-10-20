#include <gp4/gp4.h>

// Copies the three doubles 0x00622c48/50/58 into the transform block at 0x00622cf8 + 0xb8/0xc0/0xc8
// (fld qword / fstp qword). Out of line so the block offsets stay relative to the base register.
static __declspec(noinline) void storeVector(uint8_t* block) {
    GP4_FIELD(double, block, 0xb8) = GP4_GLOBAL(double, 0x00622c48);
    GP4_FIELD(double, block, 0xc0) = GP4_GLOBAL(double, 0x00622c50);
    GP4_FIELD(double, block, 0xc8) = GP4_GLOBAL(double, 0x00622c58);
}

// The three doubles at +0xa0/+0xa8/+0xb0 of the block go back to 0x00622c48/50/58.
static __declspec(noinline) void loadVector(uint8_t* block) {
    GP4_GLOBAL(double, 0x00622c48) = GP4_FIELD(double, block, 0xa0);
    GP4_GLOBAL(double, 0x00622c50) = GP4_FIELD(double, block, 0xa8);
    GP4_GLOBAL(double, 0x00622c58) = GP4_FIELD(double, block, 0xb0);
}

// Rebuilds the two rotation slots (0x20 and 0x40) from the angle triples at 0x00638dd0 and
// 0x00638030 (0x00402655), pushes the vector at 0x00622c48 through the block at 0x00622cf8
// (0x00401f02) and back, finishes the frame (0x00402564, 0x00402777 with slot 0x60) and stores the
// three binary angles left at 0x00622bd0/bd4/bd8 (0x00402a9a) into esi +0x10/+0x0c/+0x14. Every
// register and flag is whatever the last callee leaves.
// FUNCTION: GP4 0x004029e4
void __cdecl FUN_004029e4(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.eax = GP4_GLOBAL(uint32_t, 0x00638dd0);
    c.ebx = GP4_GLOBAL(uint32_t, 0x00638dd4);
    c.ecx = GP4_GLOBAL(uint32_t, 0x00638dd8);
    c.edx = 0x20u;
    gp4::call_regs(0x00402655, c);

    c.eax = GP4_GLOBAL(uint32_t, 0x00638030);
    c.ebx = GP4_GLOBAL(uint32_t, 0x00638034);
    c.ecx = GP4_GLOBAL(uint32_t, 0x00638038);
    c.edx = 0x40u;
    gp4::call_regs(0x00402655, c);

    c.edx = 0x00622cf8u;
    storeVector(reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.edx)));
    gp4::call_regs(0x00401f02, c);
    loadVector(reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.edx)));

    gp4::call_regs(0x00402564, c);
    c.edx = 0x60u;
    gp4::call_regs(0x00402777, c);
    gp4::call_regs(0x00402a9a, c);

    uint8_t* const dest = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.esi));
    c.eax = GP4_GLOBAL(uint32_t, 0x00622bd0);
    GP4_FIELD(uint32_t, dest, 0x10) = c.eax;
    c.eax = GP4_GLOBAL(uint32_t, 0x00622bd4);
    GP4_FIELD(uint32_t, dest, 0x0c) = c.eax;
    c.eax = GP4_GLOBAL(uint32_t, 0x00622bd8);
    GP4_FIELD(uint32_t, dest, 0x14) = c.eax;

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x004029e4, FUN_004029e4, "regs(esi:ptr[0x20]:bytes) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:f64")
