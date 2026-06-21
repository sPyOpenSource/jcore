import json, os, sys

def load_json(path):
    with open(path) as f:
        return json.load(f)

def make_node(file_entry, extract_entry=None):
    fp = file_entry["path"]
    cat = file_entry["fileCategory"]
    name = fp.split("/")[-1]

    if cat == "code":
        if fp.endswith(".h") or fp.endswith(".hpp"):
            ntype = "header:" + fp
        else:
            ntype = "module:" + fp
    elif cat == "docs":
        ntype = "document:" + fp
    elif cat == "config":
        ntype = "config:" + fp
    elif cat == "infra":
        if "Makefile" in name or name.endswith(".mk"):
            ntype = "pipeline:" + fp
        else:
            ntype = "service:" + fp
    elif cat == "markup":
        ntype = "file:" + fp
    elif cat == "data":
        ntype = "table:" + fp
    elif cat == "script":
        ntype = "file:" + fp
    else:
        ntype = "file:" + fp

    # Summarize based on available data
    if extract_entry:
        funcs = extract_entry.get("functions") or extract_entry.get("metrics", {}).get("functionCount", 0)
        if isinstance(funcs, list):
            fnames = [f["name"] for f in funcs[:5]]
            summary = f"Contains {len(fnames)} functions: {', '.join(fnames)}" if fnames else "Header/interface definition"
        else:
            summary = f"{funcs} functions defined"
        complexity = extract_entry.get("totalLines", 0) // 10
        totalLines = extract_entry.get("totalLines", 0)
    else:
        summary = f"{cat} file - {name}"
        complexity = file_entry.get("sizeLines", 0) // 10
        totalLines = file_entry.get("sizeLines", 0)

    # Tags
    ext = name.split(".")[-1] if "." in name else "noext"
    tags = [cat, ext]
    if cat == "code" and (fp.endswith(".h") or fp.endswith(".hpp")):
        tags.append("header")
    elif cat == "code":
        tags.append("implementation")
    if "test" in fp:
        tags.append("test")
    if "sample" in fp:
        tags.append("sample")
    if "addon" in fp:
        tags.append("addon")
    if "include/" in fp:
        tags.append("public-api")
    # ensure 3-5 tags
    while len(tags) < 3:
        tags.append(cat)
    tags = tags[:5]

    return {
        "id": fp,
        "type": ntype,
        "name": name,
        "filePath": fp,
        "summary": summary,
        "tags": tags,
        "complexity": complexity
    }

def make_edges_from_imports(batch_input, batch_extract):
    edges = []
    import_data = batch_input.get("batchImportData", {})
    for src_path, imports in import_data.items():
        for tgt in imports:
            # Only add edges for resolved relative paths (skip system headers)
            if tgt.startswith("include/") or tgt.startswith("addon/") or tgt.startswith("lib/") or tgt.startswith("test/") or tgt.startswith("sample/"):
                edges.append({
                    "source": src_path,
                    "target": tgt,
                    "type": "imports",
                    "direction": "forward",
                    "weight": 0.7
                })
    return edges

def make_cross_category_edges(batch_input, batch_extract):
    edges = []
    for f in batch_input["batchFiles"]:
        fp = f["path"]
        cat = f["fileCategory"]
        name = fp.split("/")[-1]

        if cat == "docs":
            # Document edges - connect to conceptually related code
            # For docs, connect to files matching topic keywords
            keywords_map = {
                "bootloader.txt": ["boot/", "lib/"],
                "classes.txt": ["include/circle/"],
                "debug.txt": ["lib/"],
                "keyboard.txt": ["usb/"],
                "memorymap.txt": ["include/"],
                "multicore.txt": ["smp/", "include/"],
                "qemu.txt": ["boot/"],
                "realtime.txt": ["sched/"],
                "screen.txt": ["graphics/", "display/"],
                "usb-plug-and-play.txt": ["usb/"],
                "dma-buffer-requirements.txt": ["dma/", "include/"],
            }
            for kw, prefixes in keywords_map.items():
                if kw in fp:
                    for f2 in batch_input["batchFiles"]:
                        for p in prefixes:
                            if f2["path"].startswith(p) and f2["fileCategory"] == "code":
                                edges.append({
                                    "source": fp,
                                    "target": f2["path"],
                                    "type": "documents",
                                    "direction": "forward",
                                    "weight": 0.5
                                })
            # Generic: README/CHANGELOG relate to everything in batch
            if "README.md" in fp or "CHANGELOG.md" in fp:
                for f2 in batch_input["batchFiles"]:
                    if f2["path"] != fp and f2["fileCategory"] == "code":
                        edges.append({
                            "source": fp,
                            "target": f2["path"],
                            "type": "documents",
                            "direction": "forward",
                            "weight": 0.5
                        })

        elif cat == "config":
            # Config files configure the code they co-locate with
            if "nbproject" in fp:
                # FlintJVM config - connect to Makefile
                for f2 in batch_input["batchFiles"]:
                    if "Makefile" in f2["path"]:
                        edges.append({
                            "source": fp,
                            "target": f2["path"],
                            "type": "configures",
                            "direction": "forward",
                            "weight": 0.6
                        })
            elif ".github" in fp:
                for f2 in batch_input["batchFiles"]:
                    if f2["path"] != fp:
                        edges.append({
                            "source": fp,
                            "target": f2["path"],
                            "type": "configures",
                            "direction": "forward",
                            "weight": 0.6
                        })
            else:
                for f2 in batch_input["batchFiles"]:
                    if f2["path"] != fp and f2["fileCategory"] in ("code", "infra"):
                        edges.append({
                            "source": fp,
                            "target": f2["path"],
                            "type": "configures",
                            "direction": "forward",
                            "weight": 0.6
                        })

        elif cat == "infra":
            if "Makefile" in name:
                # Makefile deploys co-located code
                prefix = "/".join(fp.split("/")[:-1]) + "/"
                for f2 in batch_input["batchFiles"]:
                    if f2["path"].startswith(prefix) and f2["path"] != fp:
                        edges.append({
                            "source": fp,
                            "target": f2["path"],
                            "type": "deploys",
                            "direction": "forward",
                            "weight": 0.7
                        })
            else:
                for f2 in batch_input["batchFiles"]:
                    if f2["path"] != fp and f2["fileCategory"] == "code":
                        edges.append({
                            "source": fp,
                            "target": f2["path"],
                            "type": "related",
                            "direction": "forward",
                            "weight": 0.5
                        })

        elif cat == "markup":
            # Markup files relate to their directory's code
            prefix = "/".join(fp.split("/")[:-1]) + "/"
            for f2 in batch_input["batchFiles"]:
                if f2["path"].startswith(prefix) and f2["path"] != fp:
                    edges.append({
                        "source": fp,
                        "target": f2["path"],
                        "type": "related",
                        "direction": "forward",
                        "weight": 0.5
                    })
    return edges

def process_batch(batch_idx):
    extract_path = f"/Users/xuyi/Source/OS/circle/.understand-anything/tmp/ua-file-extract-results-{batch_idx}.json"
    input_path = f"/Users/xuyi/Source/OS/circle/.understand-anything/tmp/ua-file-analyzer-input-{batch_idx}.json"
    output_path = f"/Users/xuyi/Source/OS/circle/.understand-anything/intermediate/batch-{batch_idx}.json"

    batch_extract = load_json(extract_path)
    batch_input = load_json(input_path)

    # Build lookup for extract results
    extract_by_path = {}
    for r in batch_extract["results"]:
        extract_by_path[r["path"]] = r

    nodes = []
    for file_entry in batch_input["batchFiles"]:
        fp = file_entry["path"]
        extract_entry = extract_by_path.get(fp)
        node = make_node(file_entry, extract_entry)
        nodes.append(node)

    edges = []
    edges.extend(make_edges_from_imports(batch_input, batch_extract))
    edges.extend(make_cross_category_edges(batch_input, batch_extract))

    output = {
        "batchIndex": batch_idx,
        "nodes": nodes,
        "edges": edges
    }

    # Only keep unique edges
    seen_edges = set()
    unique_edges = []
    for e in edges:
        key = (e["source"], e["target"], e["type"])
        if key not in seen_edges:
            seen_edges.add(key)
            unique_edges.append(e)
    output["edges"] = unique_edges

    with open(output_path, "w") as f:
        json.dump(output, f, indent=2)

    return len(nodes), len(unique_edges)

total_nodes = 0
total_edges = 0
for i in range(33, 49):
    n, e = process_batch(i)
    total_nodes += n
    total_edges += e
    print(f"Batch {i}: {n} nodes, {e} edges")

print(f"\nTotal: {total_nodes} nodes, {total_edges} edges across batches 33-48")
