#include <gp4/gp4.h>

enum : uint32_t { kArithFlags = 0x8d5u };

// Each segment locator gets its own call site so the optimizer cannot merge the two calls into one
// call_regs with a selected target address.
static __declspec(noinline) void LocateSegmentFar(gp4::Regs& s) { gp4::call_regs(0x00424e41, s); }
static __declspec(noinline) void LocateSegmentNear(gp4::Regs& s) { gp4::call_regs(0x00424e1b, s); }

// FUNCTION: GP4 0x00424e7f
// Steps the 64 object records (0x78 bytes each, array 0x0063bd7c) whose slot byte [0x0063bcfc + k] is 1.
// For each active object (esi = record, ecx = k): clears [0x0063963c] / [0x0060827d], sets [0x0063963d] to
// 0xff, runs the object setup 0x00424462, then the segment locator 0x00424e41 when [0x0062794b] != 0 or
// 0x00424e1b otherwise; a nonzero dword +0x60 that is not after the clock [0x0066d814]
// ([0x0066d814] - dword +0x60 >= 0) removes the object (0x00422833 with eax = k). One register block is
// threaded through the callees; the final flags are those of `cmp ecx, 0x40` (equal).
void __cdecl FUN_00424e7f(gp4::Regs* r) {
    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    s.st_xin_mask = 0;
    s.st_xout_mask = 0;

    s.esi = 0x0063bd7cu;
    s.ecx = 0u;
    do {
        if (GP4_ARRAY(const uint8_t, 0x0063bcfc)[s.ecx] == 1u) {
            GP4_GLOBAL(uint8_t, 0x0063963c) = 0u;
            GP4_GLOBAL(uint8_t, 0x0060827d) = 0u;
            GP4_GLOBAL(uint8_t, 0x0063963d) = 0xffu;
            gp4::call_regs(0x00424462, s);
            if (GP4_GLOBAL(uint8_t, 0x0062794b) != 0u) {
                LocateSegmentFar(s);
            } else {
                LocateSegmentNear(s);
            }
            const uint32_t deadline = GP4_FIELD(uint32_t, s.esi, 0x60);
            if (deadline != 0u) {
                const uint32_t remaining = GP4_GLOBAL(uint32_t, 0x0066d814) + (0u - deadline);
                if ((remaining >> 31) == 0u) {
                    s.eax = s.ecx;
                    gp4::call_regs(0x00422833, s);
                }
            }
        }
        s.esi += 0x78u;
        s.ecx += 1u;
    } while (s.ecx != 0x40u);

    r->eax = s.eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->esi = s.esi;
    r->edi = s.edi;
    r->ebp = s.ebp;
    // The loop ends on `cmp ecx, 0x40` with equal operands: PF and ZF set, the rest clear.
    r->eflags = (r->eflags & ~kArithFlags) | 0x44u;
}
GP4_IMPL(0x00424e7f, FUN_00424e7f, "regs(eax:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trackgeo_008943a0+tbl_00868598+state_00406b0a+ring_00406ac7+bounds_00406a57+ring_00405a12+links_00405a12+gridlinks_008943a0+state_0040759e+trackfar_008943a0+seg_00424e1b+seg_0043b127+rate_00603854+state_0042458b+blk_00424553+tag_0042456e+view_00609860+state_0042430b+state_00422ac2+state_00424011+state_00422397+recs_00621aa0+state_004250b9+state_004250f1+state_00422bc9+state_00421fc3+state_0042320a+divide_00609834+state_004240ff+gain_004240ff+state_00421a06+objs_00424e7f")
