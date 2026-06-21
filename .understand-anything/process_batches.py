#!/usr/bin/env python3
"""Process batches 17-32 to generate intermediate knowledge graph nodes and edges."""

import json
import os
import re

TMP_DIR = "/Users/xuyi/Source/OS/circle/.understand-anything/tmp"
OUT_DIR = "/Users/xuyi/Source/OS/circle/.understand-anything/intermediate"

COMPLEXITY_THRESHOLDS = {"simple": 100, "moderate": 400}

def classify_complexity(total_lines):
    if total_lines <= COMPLEXITY_THRESHOLDS["simple"]:
        return "simple"
    elif total_lines <= COMPLEXITY_THRESHOLDS["moderate"]:
        return "moderate"
    return "complex"


def path_to_tags(file_path):
    parts = file_path.replace("\\", "/").split("/")
    tags = set()
    for p in parts:
        if p in ("include", "lib", "sample", "test", "addon"):
            continue
        if p.endswith(".h") or p.endswith(".cpp"):
            p = p.rsplit(".", 1)[0]
        if p and p != parts[-1]:
            tags.add(p.lower())
    file_stem = parts[-1].rsplit(".", 1)[0] if "." in parts[-1] else parts[-1]
    tags.add(file_stem.lower())
    return sorted(tags)[:6]


def infer_summary(file_path, total_lines, functions, classes, exports):
    parts = file_path.replace("\\", "/").split("/")
    stem = parts[-1].rsplit(".", 1)[0] if "." in parts[-1] else parts[-1]

    # Map known stems to descriptions
    known = {
        "qemu": "Semihosting interface for QEMU emulation environment",
        "bcm54213": "Broadcom BCM54213 Ethernet PHY driver implementation",
        "bcmmailbox": "Mailbox interface for VideoCore property tag communication",
        "bcmpciehostbridge": "PCIe host bridge driver for Raspberry Pi 5 (BCM2712)",
        "bcmrandom": "Hardware random number generator driver for BCM2835 SoC",
        "bcmrandom200": "Hardware random number generator driver for BCM2712 (RP200)",
        "bcmwatchdog": "Hardware watchdog timer driver for Raspberry Pi",
        "smimaster": "SMI (Secondary Memory Interface) master controller driver",
        "dmasoundbuffers": "DMA sound buffer descriptor and channel management",
        "hdmisoundbasedevice": "HDMI sound output device driver via VideoCore",
        "i2ssoundbasedevice": "I2S sound output device driver",
        "i2ssoundbasedevice-rp1": "I2S sound output device driver for RP1 chip",
        "pwmsoundbasedevice": "PWM sound output device driver",
        "pwmsoundbasedevice-rp1": "PWM sound output device driver for RP1 chip",
        "southbridge": "Southbridge controller driver for PCIe-based chipset",
        "sysinit": "System initialization and early hardware setup",
        "terminal": "Terminal emulation with VT100-compatible escape sequence handling",
        "timer": "System timer interrupt handler and timekeeping",
        "smsc951x": "SMSC LAN951x USB Ethernet controller driver",
        "usbfloppydevice": "USB floppy disk drive device driver",
        "usbmassdevice": "USB bulk-only mass storage device driver",
        "xhcidevice": "xHCI (USB 3.0) host controller driver",
        "usertimer": "User-accessible kernel timer registration and management",
        "writebuffer": "Write buffering utility for coalesced output operations",
        "i2cshell": "I2C bus exploration and testing shell application",
        "soundshell": "Sound device testing and controller configuration shell",
    }

    if stem in known:
        return known[stem]

    # Generic fallback based on path and content
    path_lower = file_path.lower()
    if "/usb/" in path_lower:
        return f"USB device driver implementation for {stem}"
    if "/sound/" in path_lower:
        return f"Audio sound device driver implementation for {stem}"
    if "/rpi/" in path_lower:
        return f"Raspberry Pi hardware driver for {stem}"
    if "/sample/" in path_lower:
        return f"Sample application demonstrating {stem} functionality"
    if "/test/" in path_lower:
        return f"Test application for {stem} functionality"

    kw = stem.replace("_", " ").replace("-", " ").title()
    n_fns = len(functions) + len(classes)
    return f"{kw} implementation with {n_fns} functions and hardware register access"


def make_node_id(ntype, file_path, name=None):
    if ntype == "file":
        return f"file:{file_path}"
    key = name.replace(":", "_").replace(" ", "_")
    return f"{ntype}:{file_path}:{key}"


def process_batch(batch_num):
    extract_path = os.path.join(TMP_DIR, f"ua-file-extract-results-{batch_num}.json")
    input_path = os.path.join(TMP_DIR, f"ua-file-analyzer-input-{batch_num}.json")

    with open(extract_path) as f:
        extract = json.load(f)
    with open(input_path) as f:
        inp = json.load(f)

    extract_by_path = {r["path"]: r for r in extract["results"]}
    batch_import_data = inp.get("batchImportData", {})

    nodes = []
    edges = []
    added_nodes = set()

    def add_node(node):
        nid = node["id"]
        if nid not in added_nodes:
            added_nodes.add(nid)
            nodes.append(node)

    for fmeta in inp["batchFiles"]:
        fpath = fmeta["path"]
        er = extract_by_path.get(fpath)
        if er is None:
            continue

        total_lines = er.get("totalLines", fmeta.get("sizeLines", 0))
        functions = er.get("functions", [])
        classes = er.get("classes", [])
        exports = er.get("exports", [])
        call_graph = er.get("callGraph", [])

        complexity = classify_complexity(total_lines)
        tags = path_to_tags(fpath)
        summary = infer_summary(fpath, total_lines, functions, classes, exports)
        file_name = fpath.split("/")[-1]

        # File node
        file_node = {
            "id": make_node_id("file", fpath),
            "type": "file",
            "name": file_name,
            "filePath": fpath,
            "summary": summary,
            "tags": tags,
            "complexity": complexity,
        }
        add_node(file_node)
        file_id = file_node["id"]

        # Class nodes
        exported_class_names = {e["name"] for e in exports}
        for cls in classes:
            cname = cls["name"]
            cid = make_node_id("class", fpath, cname)
            cnode = {
                "id": cid,
                "type": "class",
                "name": cname,
                "filePath": fpath,
                "summary": f"{cname} class in {file_name}",
                "tags": tags[:4],
                "complexity": complexity,
            }
            add_node(cnode)

            # contains edge
            edges.append({
                "source": file_id,
                "target": cid,
                "type": "contains",
                "direction": "forward",
                "weight": 1.0,
            })
            # exports edge if class is exported
            if cname in exported_class_names:
                edges.append({
                    "source": file_id,
                    "target": cid,
                    "type": "exports",
                    "direction": "forward",
                    "weight": 1.0,
                })

        # Function nodes
        exported_fn_names = {e["name"] for e in exports}
        fn_name_to_id = {}
        for fn in functions:
            fname = fn["name"]
            fid = make_node_id("function", fpath, fname)
            fn_name_to_id[fname] = fid
            params_str = ", ".join(fn.get("params", []))
            fn_node = {
                "id": fid,
                "type": "function",
                "name": fname,
                "filePath": fpath,
                "summary": f"{fname}({params_str}) in {file_name}",
                "tags": tags[:3],
                "complexity": "simple" if len(fn.get("params", [])) <= 2 else "moderate",
            }
            add_node(fn_node)

            # contains edge
            edges.append({
                "source": file_id,
                "target": fid,
                "type": "contains",
                "direction": "forward",
                "weight": 1.0,
            })
            # exports edge
            if fname in exported_fn_names:
                edges.append({
                    "source": file_id,
                    "target": fid,
                    "type": "exports",
                    "direction": "forward",
                    "weight": 1.0,
                })

        # imports edges from batchImportData
        if fpath in batch_import_data:
            for imp in batch_import_data[fpath]:
                imp_id = make_node_id("file", imp)
                edges.append({
                    "source": file_id,
                    "target": imp_id,
                    "type": "imports",
                    "direction": "forward",
                    "weight": 0.7,
                })

        # calls edges from callGraph
        fn_name_to_id_in_file = {}
        for fn in functions:
            fn_name_to_id_in_file[fn["name"]] = make_node_id("function", fpath, fn["name"])

        for call_entry in call_graph:
            caller_name = call_entry["caller"]
            callee_name = call_entry["callee"]
            caller_id = fn_name_to_id_in_file.get(caller_name)
            if caller_id:
                # Try to find callee - could be in same file or other files
                callee_id = make_node_id("function", fpath, callee_name)
                # We only create calls edges for within-file calls to known functions
                if caller_id and callee_name in fn_name_to_id_in_file:
                    callee_id = fn_name_to_id_in_file[callee_name]
                    edges.append({
                        "source": caller_id,
                        "target": callee_id,
                        "type": "calls",
                        "direction": "forward",
                        "weight": 0.8,
                    })

    # Deduplicate edges
    seen_edges = set()
    unique_edges = []
    for e in edges:
        key = (e["source"], e["target"], e["type"])
        if key not in seen_edges:
            seen_edges.add(key)
            unique_edges.append(e)

    output = {
        "batch": batch_num,
        "nodes": nodes,
        "edges": unique_edges,
    }

    out_path = os.path.join(OUT_DIR, f"batch-{batch_num}.json")
    with open(out_path, "w") as f:
        json.dump(output, f, indent=2)

    return len(nodes), len(unique_edges)


def main():
    total_nodes = 0
    total_edges = 0
    for i in range(17, 33):
        n, e = process_batch(i)
        total_nodes += n
        total_edges += e
        print(f"batch-{i}: {n} nodes, {e} edges")

    # Write combined summary
    summary = {
        "totalBatches": 16,
        "batchRange": "17-32",
        "totalNodes": total_nodes,
        "totalEdges": total_edges,
    }
    with open(os.path.join(OUT_DIR, "summary-17-32.json"), "w") as f:
        json.dump(summary, f, indent=2)

    print(f"\nTotal: {total_nodes} nodes, {total_edges} edges across batches 17-32")


if __name__ == "__main__":
    main()
