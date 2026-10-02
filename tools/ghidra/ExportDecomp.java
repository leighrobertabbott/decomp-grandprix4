// Ghidra headless post-script: decompile gp4re's function starts to JSON lines.
//
//   analyzeHeadless <proj_dir> GP4 -process GP4.exe -noanalysis -scriptPath tools/ghidra
//       -postScript ExportDecomp.java <starts.txt> <out.jsonl>
//
// gp4re's function list drives the ledger, so a function is created at any start
// Ghidra's own analysis did not find; the pseudocode then lines up with the work
// items agents claim. Output is appended per function so a long run can be
// imported while it is still going.
// @category gp4re
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.decompiler.DecompiledFunction;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressSpace;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionManager;

import java.io.BufferedWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.List;

public class ExportDecomp extends GhidraScript {

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 2) {
            printerr("usage: ExportDecomp.java <starts.txt> <out.jsonl>");
            return;
        }
        List<String> lines = Files.readAllLines(Paths.get(args[0]));
        AddressSpace space = currentProgram.getAddressFactory().getDefaultAddressSpace();
        FunctionManager fm = currentProgram.getFunctionManager();
        DecompInterface ifc = new DecompInterface();
        ifc.openProgram(currentProgram);
        int done = 0, failed = 0;
        try (BufferedWriter out = Files.newBufferedWriter(Paths.get(args[1]), StandardCharsets.UTF_8,
                StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING)) {
            for (String line : lines) {
                line = line.trim();
                if (line.isEmpty()) {
                    continue;
                }
                if (monitor.isCancelled()) {
                    break;
                }
                long a = Long.parseLong(line, 16);
                Address addr = space.getAddress(a);
                Function f = fm.getFunctionAt(addr);
                if (f == null) {
                    try {
                        f = createFunction(addr, null);
                    }
                    catch (Exception e) {
                        f = null;
                    }
                }
                if (f == null) {
                    failed++;
                    continue;
                }
                DecompileResults res = ifc.decompileFunction(f, 30, monitor);
                if (res == null || !res.decompileCompleted() || res.getDecompiledFunction() == null) {
                    failed++;
                    continue;
                }
                DecompiledFunction df = res.getDecompiledFunction();
                out.write("{\"addr\":" + a + ",\"name\":" + q(f.getName()) + ",\"signature\":" +
                    q(df.getSignature()) + ",\"c\":" + q(df.getC()) + "}\n");
                if (++done % 250 == 0) {
                    out.flush();
                    println("gp4re export: " + done + " functions");
                }
            }
        }
        ifc.dispose();
        println("gp4re export: wrote " + done + " functions (" + failed + " not decompiled)");
    }

    private static String q(String s) {
        if (s == null) {
            return "null";
        }
        StringBuilder b = new StringBuilder(s.length() + 16).append('"');
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '"': b.append("\\\""); break;
                case '\\': b.append("\\\\"); break;
                case '\n': b.append("\\n"); break;
                case '\r': b.append("\\r"); break;
                case '\t': b.append("\\t"); break;
                default:
                    if (c < 0x20) {
                        b.append(String.format("\\u%04x", (int) c));
                    }
                    else {
                        b.append(c);
                    }
            }
        }
        return b.append('"').toString();
    }
}
