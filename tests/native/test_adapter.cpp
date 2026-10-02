// Native x86 test of the register-convention bridge in both directions:
//   original asm routine  --gp4_call_regs-->  gp4::Regs        (reconstruction calling an original)
//   caller registers      --regs thunk----->  reconstruction   (hook DLL replacing an original)
// Build + run: python -m tests.native  (or see tests/test_pipeline.py)
#include <stdio.h>
#include <string.h>
#include <gp4/gp4.h>
#include "../../hook/src/adapter.h"

// "original": edx -> double[2], esi = k, st0 = x  =>  eax = 3k, st0 = x*p[0]+p[1], CF = (k < 10u)
__declspec(naked) void orig_routine() {
    __asm {
        lea eax, [esi+esi*2]
        fmul qword ptr [edx]
        fadd qword ptr [edx+8]
        cmp esi, 10
        ret
    }
}

void __cdecl recon_routine(gp4::Regs* r) {
    const double* p = reinterpret_cast<const double*>(static_cast<uintptr_t>(r->edx));
    uint32_t k = r->esi;
    r->eax = k * 3;
    r->st[0] = r->st[0] * p[0] + p[1];
    r->eflags = (r->eflags & ~gp4::CF) | (k < 10u ? gp4::CF : 0u);
}

// Consume every arithmetic flag before any instruction can change it.
__declspec(naked) void orig_flags() {
    __asm { pushfd }
    __asm { pop eax }
    __asm { and eax, 0x8D5 }
    __asm { ret }
}

void __cdecl recon_flags(gp4::Regs* r) { r->eax = r->eflags & 0x8D5; }

uint32_t call_flags(uint32_t target, uint32_t input_flags) {
    uint32_t result;
    __asm {
        push input_flags
        popfd
        call target
        mov result, eax
    }
    return result;
}

struct Out { uint32_t eax, ebx, ecx, edi, esi, flags; double st0; };

Out call_machine(uint32_t target, double* p, uint32_t k, double x) {
    Out o;
    uint32_t a, b, c, d, s, f;
    double st0v;
    __asm {
        push ebx
        push esi
        push edi
        mov ebx, 0x11111111
        mov ecx, 0x22222222
        mov edi, 0x33333333
        mov edx, p
        mov esi, k
        fld x
        call target
        pushfd
        pop f
        fstp st0v
        mov a, eax
        mov b, ebx
        mov c, ecx
        mov d, edi
        mov s, esi
        pop edi
        pop esi
        pop ebx
    }
    o.eax = a; o.ebx = b; o.ecx = c; o.edi = d; o.esi = s; o.flags = f; o.st0 = st0v;
    return o;
}

int failures = 0;
#define CHECK(c, ...) do { if (!(c)) { ++failures; printf("FAIL: " __VA_ARGS__); printf("\n"); } } while (0)

int main() {
    gp4::ImplRecord flag_rec = {0, (const void*)&recon_flags, "recon_flags", "regs() -> (eax)"};
    uint32_t flag_thunk = make_regs_thunk(flag_rec);
    const uint32_t flag_bits[] = {1, 4, 16, 64, 128, 2048};
    for (uint32_t mask = 0; mask < 64; ++mask) {
        uint32_t flags = 0x202;
        for (uint32_t bit = 0; bit < 6; ++bit)
            if (mask & (1u << bit)) flags |= flag_bits[bit];
        uint32_t expected = flags & 0x8D5;
        CHECK(call_flags((uint32_t)&orig_flags, flags) == expected, "original input flags mask=%u", mask);
        CHECK(call_flags(flag_thunk, flags) == expected, "hook input flags mask=%u", mask);
        gp4::Regs flag_regs = {};
        flag_regs.eflags = flags;
        gp4::call_regs((uintptr_t)&orig_flags, flag_regs);
        CHECK(flag_regs.eax == expected, "gp4_call_regs input flags mask=%u", mask);
    }
    gp4::ImplRecord rec = {0, (const void*)&recon_routine, "recon_routine",
                           "regs(edx:ptr[16]:f64, esi:u32, st0:f64) -> (eax, st0, cf)"};
    uint32_t thunk = make_regs_thunk(rec);
    double p[2] = {1.75, -0.125};
    uint32_t ks[] = {0, 3, 9, 10, 11, 1000};
    double xs[] = {0.0, 1.0, -2.5, 123.456};
    for (uint32_t k : ks) for (double x : xs) {
        Out a = call_machine((uint32_t)&orig_routine, p, k, x);
        Out b = call_machine(thunk, p, k, x);
        CHECK(a.eax == b.eax, "eax k=%u: %08x vs %08x", k, a.eax, b.eax);
        CHECK(a.st0 == b.st0, "st0 k=%u x=%g: %.17g vs %.17g", k, x, a.st0, b.st0);
        CHECK((a.flags & 1) == (b.flags & 1), "CF k=%u", k);
        CHECK(a.ebx == b.ebx && a.ecx == b.ecx && a.edi == b.edi && a.esi == b.esi, "preserved regs k=%u", k);

        gp4::Regs r = {};
        r.edx = (uint32_t)(uintptr_t)p; r.esi = k; r.st[0] = x; r.st_in = 1; r.st_out = 1;
        gp4::Regs r2 = r;
        gp4::call_regs((uintptr_t)&orig_routine, r);
        recon_routine(&r2);
        CHECK(r.eax == r2.eax && r.st[0] == r2.st[0] && (r.eflags & 1) == (r2.eflags & 1),
              "gp4_call_regs k=%u x=%g: eax %08x/%08x st0 %.17g/%.17g", k, x, r.eax, r2.eax, r.st[0], r2.st[0]);
    }
    printf(failures ? "%d FAILURES\n" : "ALL PASS (%d)\n", failures ? failures : 88);
    return failures ? 1 : 0;
}
