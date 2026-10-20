"""Small, real-Unicorn counterexamples for the acceptance oracle.

Synthetic images contain no game assets and require neither MSVC nor analysis
state. Each failure exercises executable x86 rather than mocking the oracle.
"""
from __future__ import annotations

import struct
import random
import unittest
from pathlib import Path
from unittest.mock import patch

from gp4re.binary import Binary, Section
from gp4re.emu import CaseGen, Impl, Machine, PAGE, _float_equal, compare_pages, differential
from gp4re.spec import parse


BASE, TARGET, INIT, DLL, IMPL = 0x400000, 0x401000, 0x527453, 0x10000000, 0x10001000


def images(original: bytes, reconstruction: bytes, data: dict[int, bytes] | None = None):
    game = bytearray(0x128000)
    game[TARGET - BASE: TARGET - BASE + len(original)] = original
    game[INIT - BASE] = 0xC3
    for addr, raw in (data or {}).items():
        game[addr - BASE: addr - BASE + len(raw)] = raw
    recon = bytearray(0x3000)
    recon[IMPL - DLL: IMPL - DLL + len(reconstruction)] = reconstruction
    code = Section(".text", TARGET, 0x127000, 0x127000, 0x60000020, 0, True)
    dcode = Section(".text", IMPL, 0x2000, 0x2000, 0x60000020, 0, True)
    return (Binary(Path("synthetic-gp4.exe"), b"", BASE, TARGET, game, [code]),
            Binary(Path("synthetic-recon.dll"), b"", DLL, IMPL, recon, [dcode]))


class OracleSoundnessTests(unittest.TestCase):
    def run_pair(self, original, reconstruction, spec, *, data=None, settings=None):
        cfg = {"init_calls": [INIT], "max_cases": 4, "time_budget_s": 2,
               "min_block_coverage": 0.90, "min_branch_coverage": 0.70, "ni_cases": 1}
        cfg.update(settings or {})
        gp4, dll = images(original, reconstruction, data)
        rec = Impl(TARGET, IMPL, "synthetic", spec)
        with patch("gp4re.config.oracle", side_effect=cfg.__getitem__):
            return differential(gp4, dll, rec, parse(spec), fpucw=0x037F, cases=2,
                                function_starts={TARGET, INIT})

    def run_with_imports(self, original, reconstruction, spec, imports):
        cfg = {"init_calls": [INIT], "max_cases": 4, "time_budget_s": 2,
               "min_block_coverage": 0.90, "min_branch_coverage": 0.70, "ni_cases": 1}
        gp4, dll = images(original, reconstruction)
        gp4.imports = dict(imports)
        rec = Impl(TARGET, IMPL, "synthetic", spec)
        with patch("gp4re.config.oracle", side_effect=cfg.__getitem__):
            return differential(gp4, dll, rec, parse(spec), fpucw=0x037F, cases=2,
                                function_starts={TARGET, INIT})

    HEAP_IAT = {0x510000: "kernel32.dll!GetProcessHeap", 0x510004: "kernel32.dll!HeapAlloc",
                0x510008: "kernel32.dll!OutputDebugStringA", 0x51000C: "kernel32.dll!Sleep"}

    @staticmethod
    def alloc_code(size: int, flags: int, tail: bytes) -> bytes:
        # call [GetProcessHeap]; push size; push flags; push eax; call [HeapAlloc]; <tail>
        return (bytes.fromhex("ff1500005100") + bytes([0x6A, size, 0x6A, flags, 0x50]) +
                bytes.fromhex("ff1504005100") + tail)

    def test_modelled_heap_alloc_runs_and_is_reported(self):
        code = self.alloc_code(0x10, 8, bytes.fromhex("c7002a000000c3"))   # mov dword [eax],42; ret
        r = self.run_with_imports(code, code, "cdecl() -> u32", self.HEAP_IAT)
        self.assertEqual(r["status"], "pass", r)
        self.assertEqual(set(r["import_models"]), {"GetProcessHeap", "HeapAlloc"})

    def test_uninitialised_heap_bytes_are_identical_for_both_runs(self):
        code = self.alloc_code(0x10, 0, bytes.fromhex("8b00c3"))           # mov eax,[eax]; ret
        r = self.run_with_imports(code, code, "cdecl() -> u32", self.HEAP_IAT)
        self.assertEqual(r["status"], "pass", r)

    def test_different_import_arguments_fail_even_with_equal_effects(self):
        original = self.alloc_code(0x10, 8, bytes.fromhex("c7002a000000c3"))
        recon = self.alloc_code(0x20, 8, bytes.fromhex("c7002a000000c3"))   # same pointer, same write
        r = self.run_with_imports(original, recon, "cdecl() -> u32", self.HEAP_IAT)
        self.assertEqual(r["status"], "fail", r)
        self.assertIn("import call #1", r["mismatch"])

    def test_debug_string_content_is_compared(self):
        def code(ch):   # push esp-relative string "<ch>\0": push imm32; push esp; call [OutputDebugStringA]; pop eax; ret
            return bytes([0x68, ch, 0, 0, 0, 0x54]) + bytes.fromhex("ff1508005100") + bytes.fromhex("58c3")
        same = self.run_with_imports(code(0x41), code(0x41), "cdecl() -> void", self.HEAP_IAT)
        self.assertEqual(same["status"], "pass", same)
        diff = self.run_with_imports(code(0x41), code(0x42), "cdecl() -> void", self.HEAP_IAT)
        self.assertEqual(diff["status"], "fail", diff)
        self.assertIn("OutputDebugStringA", diff["mismatch"])

    def test_an_extra_import_call_fails(self):
        original = bytes.fromhex("c3")
        recon = bytes.fromhex("6a00ff150c005100c3")   # push 0; call [Sleep]; ret
        r = self.run_with_imports(original, recon, "cdecl() -> void", self.HEAP_IAT)
        self.assertEqual(r["status"], "fail", r)
        self.assertIn("only the reconstruction", r["mismatch"])

    def test_file_models_are_a_consistent_seeded_file_system(self):
        from gp4re.stubs import ImportStubs, INVALID_HANDLE

        class FakeUc:   # flat memory at 0x1000..0x9000 for the model methods
            def __init__(self):
                self.mem = bytearray(0x8000)
            def mem_read(self, a, n):
                return self.mem[a - 0x1000:a - 0x1000 + n]
            def mem_write(self, a, data):
                self.mem[a - 0x1000:a - 0x1000 + len(data)] = data

        def session(seed):
            uc = FakeUc()
            uc.mem_write(0x1000, b"race.dat\0")
            s = ImportStubs(uc, lambda a, n: None, (0x7000, 0x9000))
            for k in range(40):          # find a seed offset where the open succeeds
                s.reset(seed + k)
                h, _ = s.create_file(0x1000, 0x80000000, 1, 0, 3, 0x80, 0)
                if h != INVALID_HANDLE:
                    break
            size, _ = s.get_file_size(h, 0)
            ok, _ = s.read_file(h, 0x2000, 0x10, 0x1f00, 0)
            got = struct.unpack("<I", bytes(uc.mem_read(0x1f00, 4)))[0]
            pos, _ = s.set_file_pointer(h, 0xFFFFFFFC, 0, 2)          # 4 bytes before the end
            closed, _ = s.close_handle(h)
            again, _ = s.read_file(h, 0x2000, 4, 0x1f00, 0)          # closed handle: fails
            return size, ok, got, pos, closed, again, bytes(uc.mem_read(0x2000, got)), list(s.log)

        a, b = session(7), session(7)
        self.assertEqual(a, b)                                           # deterministic per seed
        size, ok, got, pos, closed, again, data, _ = a
        self.assertEqual((ok, closed, again), (1, 1, 0))
        self.assertEqual(got, min(0x10, size))
        self.assertEqual(pos, size - 4 if size >= 4 else INVALID_HANDLE)

    def test_preopened_handles_are_seeded_files_and_others_are_invalid(self):
        from gp4re.stubs import ImportStubs, INVALID_HANDLE

        class FakeUc:
            def __init__(self):
                self.mem = bytearray(0x100)
            def mem_read(self, a, n):
                return self.mem[a:a + n]
            def mem_write(self, a, d):
                self.mem[a:a + len(d)] = d

        s = ImportStubs(FakeUc(), lambda a, n: None, (0x80, 0x100))
        size, _ = s.get_file_size(0x3000, 0)            # opened by a caller before this run
        self.assertNotEqual(size, INVALID_HANDLE)
        self.assertEqual(s.close_handle(0x3000)[0], 1)
        self.assertEqual(s.get_file_size(0x3000, 0)[0], INVALID_HANDLE)   # closed stays closed
        self.assertEqual(s.get_file_size(0x5000, 0)[0], INVALID_HANDLE)   # never opened anywhere

    def test_written_file_content_is_logged(self):
        from gp4re.stubs import ImportStubs

        class FakeUc:
            def __init__(self, data):
                self.mem = bytearray(0x100); self.mem[:len(data)] = data
            def mem_read(self, a, n):
                return self.mem[a:a + n]
            def mem_write(self, a, d):
                self.mem[a:a + len(d)] = d

        logs = []
        for content in (b"lap1", b"lap2"):
            s = ImportStubs(FakeUc(content), lambda a, n: None, (0x80, 0x100))
            s.files[0x1000] = __import__("gp4re.stubs", fromlist=["_File"])._File(b"")
            logs.append(s.write_file(0x1000, 0, 4, 0x40, 0)[1])
        self.assertNotEqual(logs[0], logs[1])

    def test_rep_movsb_keeps_the_flags_of_the_preceding_add(self):
        # mov ecx,1; mov eax,0xf; add eax,1 (AF=1); rep movsb; ret - rep movs never writes flags,
        # but Unicorn with hooks installed used to lose AF/CF across it.
        from gp4re.emu import SCRATCH
        game, _ = images(bytes.fromhex("b901000000b80f00000083c001f3a4c3"), b"\xc3")
        machine = Machine(game, None, fpucw=0x037F)
        machine.reset()
        result = machine.call(TARGET, regs={"esi": SCRATCH, "edi": SCRATCH + 0x100}, stack=b"", st_in=[],
                              n_st_out=0, capture_flags=True, maxinsn=100)
        self.assertEqual(result.status, "ok")
        self.assertEqual(result.flags & 0x8D5, 0x010)   # AF set, CF/PF/ZF/SF/OF clear (0x10 = 0x0f + 1)

    def test_original_operand_code_refs_are_function_starts_only(self):
        from gp4re.shape import original_code_refs
        # mov esi,0x402000; add esi,0x403000; mov edi,0x401800; ret
        gp4, _ = images(bytes.fromhex("be0020400081c600304000bf00184000c3"), b"\xc3")
        refs = original_code_refs(gp4, TARGET, {TARGET, 0x402000, 0x403000})
        self.assertEqual(refs, {0x402000})   # arithmetic operands and non-starts are numbers

    def test_add_table_switch_case_bodies_are_in_the_coverage_cfg(self):
        from gp4re.emu import target_coverage
        # cmp eax,2; jb +5; mov eax,1; shl eax,2; add eax,TABLE; jmp [eax]; TABLE: case0, case1;
        # case0: mov ecx,1; ret   case1: mov ecx,2; ret
        table, case0, case1 = TARGET + 20, TARGET + 28, TARGET + 34
        code = (bytes.fromhex("83f8027205b801000000c1e002") + b"\x05" + struct.pack("<I", table)
                + b"\xff\x20" + struct.pack("<II", case0, case1)
                + bytes.fromhex("b901000000c3b902000000c3"))
        gp4, _ = images(code, b"\xc3")
        cov = target_coverage(gp4, TARGET, {TARGET})
        self.assertTrue({case0, case1} <= set(cov.blocks), sorted(map(hex, cov.blocks)))
        self.assertNotIn(table, cov.blocks)

    def test_emu_skip_still_compares_cases_that_never_reach_the_api(self):
        # original: mov eax,42; ret   reconstruction: ret  -- no import is ever called, so the
        # runtime-only exception must not hide the eax mismatch
        r = self.run_pair(bytes.fromhex("b82a000000c3"), b"\xc3", "regs() -> (eax); emu=skip:win32 PeekMessageA")
        self.assertEqual(r["status"], "fail", r)
        same = self.run_pair(bytes.fromhex("b82a000000c3"), bytes.fromhex("8b442404c7002a000000c3"),
                             "regs() -> (eax); emu=skip:win32 PeekMessageA")
        self.assertEqual(same["status"], "skipped", same)
        self.assertIn("unnecessary", same.get("note", ""))

    def test_omitted_register_output_cannot_accept_noop(self):
        r = self.run_pair(bytes.fromhex("b82a000000c3"), b"\xc3", "regs() -> ()")
        self.assertEqual(r["status"], "fail", r)
        self.assertIn("eax:", r["mismatch"])

    def test_reconstruction_cannot_clobber_undeclared_preserved_register(self):
        # mov eax,[esp+4]; mov dword ptr[eax+4],42; ret -- overwrite r.ebx
        r = self.run_pair(b"\xc3", bytes.fromhex("8b442404c740042a000000c3"), "regs() -> ()")
        self.assertEqual(r["status"], "fail", r)
        self.assertIn("ebx:", r["mismatch"])

    def test_missing_input_cannot_be_replaced_with_canary_constant(self):
        r = self.run_pair(bytes.fromhex("89f8c3"), bytes.fromhex("b80005dec0c3"), "cdecl() -> u32")
        self.assertEqual(r["status"], "fail", r)
        self.assertIn("hidden input edi", r["mismatch"])

    def test_hidden_carry_input_is_detected(self):
        # setb al; movzx eax,al; ret
        code = bytes.fromhex("0f92c00fb6c0c3")
        r = self.run_pair(code, code, "cdecl() -> u32")
        self.assertEqual(r["status"], "fail", r)
        self.assertIn("hidden input cf", r["mismatch"])

    def test_declared_carry_input_is_generated(self):
        # Original mov eax,0; adc eax,0 has a declared incoming carry. The
        # reconstruction implements it in r.eax and computes arithmetic flags.
        original = bytes.fromhex("b80000000083d000c3")
        # mov edx,[esp+4]; push [edx+0x1c]; popfd; mov eax,0; adc eax,0;
        # mov [edx],eax; pushfd; pop eax; mov [edx+0x1c],eax; ret
        recon = bytes.fromhex("8b542404ff721c9db80000000083d00089029c5889421cc3")
        # edx is restored from the input block, so the native local register is
        # irrelevant; original does not modify its incoming edx.
        r = self.run_pair(original, recon, "regs(cf:u32[0..1]) -> (eax,cf,pf,af,zf,sf,of)")
        self.assertEqual(r["status"], "pass", r)

    def test_related_register_input_is_generated(self):
        # mov eax,ebp; sub eax,edi; ret  -- with ebp = edi + k*0x10 the difference is k*0x10
        original = bytes.fromhex("89e829f8c3")
        # mov edx,[esp+4]; mov eax,[edx+0x18]; sub eax,[edx+0x14]; mov [edx],eax; ret
        recon = bytes.fromhex("8b5424048b42182b42148902c3")
        r = self.run_pair(original, recon, "regs(edi:u32, ebp:rel[edi,0x10,1..3]) -> (eax)")
        self.assertEqual(r["status"], "pass", r)
        # mov edx,[esp+4]; mov dword ptr [edx],0x10; ret -- right only for k == 1
        wrong = bytes.fromhex("8b542404c70210000000c3")
        r = self.run_pair(original, wrong, "regs(edi:u32, ebp:rel[edi,0x10,5..5]) -> (eax)")
        self.assertEqual(r["status"], "fail", r)

    def test_hidden_input_on_a_rare_path_is_detected(self):
        # cmp ecx,0x12345678; jne +3; mov eax,edx; ret; mov eax,1; ret -- edx matters only on the rare path
        original = bytes.fromhex("81f978563412" "7503" "89d0c3" "b801000000c3")
        # recon: r->eax = (r->ecx == 0x12345678) ? r->edx : 1, flags of the cmp
        recon = bytes.fromhex("8b542404" "8b4a08" "b801000000" "81f978563412" "7503" "8b420c" "8902"
                              "9c" "58" "89421c" "c3")
        r = self.run_pair(original, recon, "regs(ecx:u32) -> (eax)",
                          settings={"max_cases": 40, "ni_cases": 1})
        self.assertEqual(r["status"], "fail", r)
        self.assertIn("hidden input edx", str(r.get("mismatch")))

    def test_signed_zero_is_bit_exact(self):
        r = self.run_pair(bytes.fromhex("d9eec3"), bytes.fromhex("d9eed9e0c3"), "cdecl() -> f64")
        self.assertEqual(r["status"], "fail", r)
        self.assertIn("return st0", r["mismatch"])

    def test_distinct_nan_payloads_are_bit_exact(self):
        data = {0x401300: struct.pack("<Q", 0x7FF8000000000001),
                0x401308: struct.pack("<Q", 0x7FF8000000000002)}
        r = self.run_pair(b"\xdd\x05" + struct.pack("<I", 0x401300) + b"\xc3",
                          b"\xdd\x05" + struct.pack("<I", 0x401308) + b"\xc3",
                          "cdecl() -> f64", data=data)
        self.assertEqual(r["status"], "fail", r)

    def test_compiled_abi_preserves_ebx(self):
        r = self.run_pair(b"\xc3", bytes.fromhex("bb2a000000c3"), "cdecl() -> void")
        self.assertEqual(r["status"], "fail", r)
        self.assertIn("callee-saved ebx", r["mismatch"])

    def test_compiled_stack_balance_matches(self):
        r = self.run_pair(b"\xc3", bytes.fromhex("c20400"), "cdecl() -> void")
        self.assertEqual(r["status"], "fail", r)
        self.assertIn("stack balance", r["mismatch"])

    def test_live_caller_argument_write_is_compared(self):
        r = self.run_pair(bytes.fromhex("c74424042a000000c3"), b"\xc3", "cdecl(i32) -> void")
        self.assertEqual(r["status"], "fail", r)
        self.assertIn("caller-owned stack", r["mismatch"])

    def test_undeclared_stack_argument_is_detected(self):
        code = bytes.fromhex("8b442404c3")
        r = self.run_pair(code, code, "cdecl() -> u32")
        self.assertEqual(r["status"], "fail", r)
        self.assertIn("hidden input undeclared stack", r["mismatch"])

    def test_uncovered_branch_cannot_pass(self):
        # Declared input is constrained to zero, so only one of the branch's
        # two outcomes is reachable by this spec. Extra search cannot help.
        code = bytes.fromhex("8b44240485c07406b82a000000c3c3")
        r = self.run_pair(code, code, "cdecl(i32[0..0]) -> u32")
        self.assertEqual(r["status"], "inconclusive", r)
        self.assertEqual(r["coverage"]["branch_fraction"], 0.5)
        self.assertEqual(r["cases"], 4)

    def test_branch_to_next_instruction_counts_as_both_directions(self):
        # mov eax,[esp+4]; test eax,eax; je +0; ret -- taken and fallthrough are
        # the same address, so neither direction can be told apart or missed.
        code = bytes.fromhex("8b44240485c07400c3")
        r = self.run_pair(code, code, "cdecl(i32[0..0]) -> u32")
        self.assertEqual(r["status"], "pass", r)
        self.assertEqual(r["coverage"]["branch_fraction"], 1.0)

    def test_dead_block_needs_a_lead_exemption(self):
        # mov eax,[esp+4]; and eax,1; cmp eax,2; jne +5; mov eax,42 (dead); ret
        code = bytes.fromhex("8b44240483e00183f8027505b82a000000c3")
        r = self.run_pair(code, code, "cdecl(i32) -> u32")
        self.assertEqual(r["status"], "inconclusive", r)
        with patch("gp4re.emu._coverage_exemptions", return_value={TARGET + 12}):
            r = self.run_pair(code, code, "cdecl(i32) -> u32")
        self.assertEqual(r["status"], "pass", r)
        self.assertEqual(r["coverage"]["exempt_blocks"], [hex(TARGET + 12)])

    def test_coverage_search_uses_branch_boundary_constants(self):
        code = bytes.fromhex("8b4424043d785634127406b82a000000c3c3")
        r = self.run_pair(code, code, "cdecl(u32) -> u32", settings={"max_cases": 80})
        self.assertEqual(r["status"], "pass", r)
        self.assertEqual(r["coverage"]["branch_fraction"], 1.0)
        self.assertGreater(r["cases"], 2)

    def test_callee_bodies_are_not_target_coverage(self):
        target = b"\xe8" + struct.pack("<i", 0x401100 - (TARGET + 5)) + b"\xc3"
        recon = b"\xe8" + struct.pack("<i", 0x401100 - (IMPL + 5)) + b"\xc3"
        # Callee has an unexercised branch; only its call site belongs to target.
        callee = bytes.fromhex("31c085c07406b82a000000c3c3")
        r = self.run_pair(target, recon, "cdecl() -> u32", data={0x401100: callee})
        self.assertEqual(r["status"], "pass", r)
        self.assertEqual(r["coverage"]["branch_directions_total"], 0)

    def test_initialization_failure_is_closed(self):
        r = self.run_pair(b"\xc3", b"\xc3", "cdecl() -> void", settings={"init_calls": [0xDEADBEEF]})
        self.assertEqual(r["status"], "fail", r)
        self.assertEqual(r["reason"], "initialization failed")

    def test_unrepresentable_extended_regs_output_is_inconclusive(self):
        # Exact x87 value 1 + 2^-63 rounds to binary64 1.0.
        data = {0x401300: struct.pack("<QH", 0x8000000000000001, 0x3FFF)}
        original = b"\xdb\x2d" + struct.pack("<I", 0x401300) + b"\xc3"
        r = self.run_pair(original, b"\xc3", "regs() -> (st0)", data=data)
        self.assertEqual(r["status"], "inconclusive", r)
        self.assertIn("a double cannot hold", r["reason"])

    def test_extended_regs_output_is_compared_bit_exact(self):
        data = {0x401300: struct.pack("<QH", 0x8000000000000001, 0x3FFF)}
        original = bytes.fromhex("db2d") + struct.pack("<I", 0x401300) + bytes.fromhex("c3")
        # mov eax,[esp+4]; fld tbyte [SRC]; fstp tbyte [eax+0xB8] (st_xout[0]);
        # or dword [eax+0x10C],1 (st_xout_mask); ret
        def recon(src):
            return (bytes.fromhex("8b442404") + bytes.fromhex("db2d") + struct.pack("<I", src) +
                    bytes.fromhex("dbb8b8000000") + bytes.fromhex("83880c01000001") + bytes.fromhex("c3"))
        r = self.run_pair(original, recon(0x401300), "regs() -> (st0)", data=data)
        self.assertEqual(r["status"], "pass", r)
        data[0x401310] = struct.pack("<QH", 0x8000000000000000, 0x3FFF)   # exactly 1.0
        r = self.run_pair(original, recon(0x401310), "regs() -> (st0)", data=data)
        self.assertEqual(r["status"], "fail", r)
        self.assertIn("st0: original 80-bit", r["mismatch"])

    def test_compiled_extended_difference_is_compared_before_rounding(self):
        data = {0x401300: struct.pack("<QH", 0x8000000000000001, 0x3FFF)}
        original = b"\xdb\x2d" + struct.pack("<I", 0x401300) + b"\xc3"
        r = self.run_pair(original, bytes.fromhex("d9e8c3"), "cdecl() -> f64", data=data)
        self.assertEqual(r["status"], "fail", r)
        self.assertIn("return st0", r["mismatch"])

    def test_st_inputs_are_loaded_in_register_order(self):
        original = bytes.fromhex("dee9c3")  # fsubp st1,st0
        recon = bytes.fromhex("8b442404c7402000000000c7402400000040c3")
        r = self.run_pair(original, recon, "regs(st1:f64[5..5],st0:f64[3..3]) -> (st0)")
        self.assertEqual(r["status"], "pass", r)

    def test_any_original_fault_keeps_verification_inconclusive(self):
        # One declared scalar value faults; valid cases cannot hide it.
        code = bytes.fromhex("8b44240485c07406a1efbeaddec3c3")
        r = self.run_pair(code, code, "cdecl(i32[0..1]) -> u32", settings={"max_cases": 20})
        self.assertEqual(r["status"], "inconclusive", r)
        self.assertGreater(r["invalid"], 0)

    def test_out_of_scope_code_cannot_execute(self):
        target = b"\xe9" + struct.pack("<i", 0x401100 - (TARGET + 5))
        gp4, dll = images(target, b"\xc3", {0x401100: b"\xc3"})
        gp4.sections.append(Section("wrapper", 0x401100, 1, 1, 0x60000020, 0, False))
        cfg = {"init_calls": [INIT], "max_cases": 2, "time_budget_s": 2,
               "min_block_coverage": 0.9, "min_branch_coverage": 0.7, "ni_cases": 1}
        rec = Impl(TARGET, IMPL, "synthetic", "cdecl() -> void")
        with patch("gp4re.config.oracle", side_effect=cfg.__getitem__):
            r = differential(gp4, dll, rec, parse(rec.spec), fpucw=0x037F, cases=2,
                             function_starts={TARGET, INIT, 0x401100})
        self.assertEqual(r["status"], "inconclusive", r)
        self.assertEqual(r["invalid_reasons"], {"fault": 2})

    def test_undeclared_x87_output_is_rejected(self):
        r = self.run_pair(bytes.fromhex("d9eec3"), b"\xc3", "cdecl() -> void")
        self.assertEqual(r["status"], "fail", r)
        self.assertIn("x87 output contract", r["mismatch"])

    def test_memory_tolerance_cannot_hide_integer_changes(self):
        a, b = bytes(PAGE), b"\x01" + bytes(PAGE - 1)
        self.assertTrue(compare_pages({0: a}, {0: b}, lambda _: a, 1e-3))

    def test_tolerance_preserves_signed_zero_and_nonfinite_bits(self):
        for left, right in ((0.0, -0.0), (float("inf"), 1.0),
                            (float("inf"), float("-inf"))):
            self.assertFalse(_float_equal(struct.pack("<d", left), struct.pack("<d", right), 1e-3))

    def test_wide_integer_ranges_include_zero_and_sign_boundaries(self):
        argument = parse("cdecl(i32[-32767..32767]) -> void").args[0].t
        generator = CaseGen(parse("cdecl() -> void"), 7)
        values = {generator.scalar(argument) for _ in range(200)}
        self.assertTrue({-32767, -1, 0, 1, 32767}.issubset(values))
        self.assertTrue(all(-32767 <= value <= 32767 for value in values))

    def test_buffer_layout_points_into_a_record_array(self):
        import json, tempfile
        from gp4re import emu as emu_mod
        with tempfile.TemporaryDirectory() as d:
            (Path(d) / "pi.json").write_text(json.dumps({"size": 16, "fields": [
                {"off": 4, "type": "u32", "points_into": {"base": "0x8943a0", "stride": "0x160", "lo": 2, "hi": 5}}]}))
            with patch.object(emu_mod, "LAYOUT_DIR", Path(d)):
                generator = CaseGen(parse("cdecl() -> void"), 3)
                seen = {struct.unpack_from("<I", generator.layout("pi", 16), 4)[0] for _ in range(80)}
        self.assertEqual(seen, {0x8943a0 + k * 0x160 for k in range(2, 6)})

    def test_buffer_layout_self_field_points_at_its_own_buffer(self):
        import json, tempfile
        from gp4re import emu as emu_mod
        with tempfile.TemporaryDirectory() as d:
            (Path(d) / "node.json").write_text(json.dumps({"size": 16, "fields": [
                {"off": 0, "type": "self"}, {"off": 8, "type": "self", "bias": 4},
                {"off": 12, "type": "u32", "values": [7]}]}))
            (Path(d) / "over.json").write_text(json.dumps({"size": 16, "fields": [
                {"off": 8, "type": "u32", "values": [9]}, {"off": 4, "type": "u32", "values": [5]}]}))
            with patch.object(emu_mod, "LAYOUT_DIR", Path(d)):
                generator = CaseGen(parse("cdecl() -> void"), 3)
                addr = generator.alloc(generator.fill(16, "layout=node"))
                both = generator.alloc(generator.fill(16, "layout=node+over"))
        data = dict(generator.buffers)
        self.assertEqual(struct.unpack_from("<4I", data[addr]), (addr, 0, addr + 4, 7))
        # a later layout overrides the self field at +8 and still applies its other fields
        self.assertEqual(struct.unpack_from("<4I", data[both]), (both, 5, 9, 7))

    def test_value_set_type_draws_its_members(self):
        spec = parse("regs(eax:u32{0x36314c20,0x36314c41}, ebx:u32) -> (eax)")
        generator = CaseGen(spec, 5)
        values = [generator.scalar(spec.args[0].t) for _ in range(200)]
        self.assertTrue({0x36314c20, 0x36314c41}.issubset(values))
        self.assertTrue(any(v not in (0x36314c20, 0x36314c41) for v in values))

    def test_unicorn_shrd12_flags_follow_penultimate_shift(self):
        # Architecture leaves OF undefined for count>1. This checks the pinned
        # oracle's model, not equality of undefined bits on every native CPU.
        game, _ = images(bytes.fromhex("0facd00cc3"), b"\xc3")
        machine = Machine(game, None, fpucw=0x037F)
        rng = random.Random(52)
        pairs = [(0, 0), (0xFFFFFFFF, 0xFFFFFFFF), (0, 0x80000000),
                 (0x80000000, 0), (0, 0x1000), (0, 0x800)]
        pairs += [(rng.getrandbits(32), rng.getrandbits(32)) for _ in range(128)]
        for low, high in pairs:
            machine.reset()
            result = machine.call(TARGET, regs={"eax": low, "edx": high}, stack=b"", st_in=[],
                                  n_st_out=0, capture_flags=True, maxinsn=100)
            self.assertEqual(result.status, "ok")
            combined = (high << 32) | low
            output = (combined >> 12) & 0xFFFFFFFF
            previous = (combined >> 11) & 0xFFFFFFFF
            overflow = ((previous ^ output) >> 31) & 1
            self.assertEqual(result.regs["eax"], output)
            self.assertEqual((result.flags >> 11) & 1, overflow)

    def test_asymmetric_dirty_page_uses_loaded_input_baseline(self):
        loaded = b"\x05" + bytes(PAGE - 1)
        self.assertEqual(compare_pages({0: loaded}, {}, lambda _: loaded, 0), [])


if __name__ == "__main__":
    unittest.main()
