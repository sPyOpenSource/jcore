#!/usr/bin/env python3
"""Process batches 49-64: read input + extraction, produce nodes+edges, write output."""
import json
import os
import re
from collections import defaultdict

BASE = "/Users/xuyi/Source/OS/circle"
TMP = f"{BASE}/.understand-anything/tmp"
OUT = f"{BASE}/.understand-anything/intermediate"
os.makedirs(OUT, exist_ok=True)


def complexity(lines):
    if lines < 50:
        return "simple"
    if lines < 200:
        return "moderate"
    return "complex"


def type_from_file(fm):
    """Determine node type and id prefix from file metadata."""
    cat = fm.get("fileCategory", "")
    path = fm.get("path", "")
    fname = os.path.basename(path)

    if cat == "code":
        return "file", "file"
    elif cat == "config":
        return "config", "config"
    elif cat == "docs":
        return "document", "document"
    elif cat == "markup":
        return "file", "file"
    elif cat == "script":
        return "file", "file"
    elif cat == "infra":
        lower = path.lower()
        if "dockerfile" in lower or fname == "Dockerfile":
            return "service", "service"
        if ".github" in path and ("ci" in lower or "workflow" in lower):
            return "pipeline", "pipeline"
        if ".tf" in fname or "terraform" in lower:
            return "resource", "resource"
        return "file", "file"
    elif cat == "data":
        lower = path.lower()
        if ".sql" in fname or "sql" in lower:
            return "table", "table"
        if ".graphql" in fname or ".proto" in fname:
            return "schema", "schema"
        if "openapi" in lower or "swagger" in lower:
            return "endpoint", "endpoint"
        return "file", "file"
    else:
        return "file", "file"


def summarize_file(path, language):
    """Generate a short summary based on path/language."""
    fname = os.path.basename(path)
    if language == "makefile":
        return f"Build system configuration for {os.path.dirname(path) or 'project root'}"
    if language == "html":
        return f"HTML document for {os.path.dirname(path) or 'project root'}"
    if language in ("css", "scss"):
        return f"Stylesheet for {os.path.dirname(path) or 'project root'}"
    if language in ("c++", "c", "cpp", "h", "hpp"):
        return f"C++ source/header in {os.path.dirname(path) or 'project root'}"
    if language == "python":
        return f"Python script in {os.path.dirname(path) or 'project root'}"
    if language == "markdown":
        return f"Documentation in {os.path.dirname(path) or 'project root'}"
    if language in ("java", "kotlin"):
        return f"{language.title()} source in {os.path.dirname(path) or 'project root'}"
    return f"File in {os.path.dirname(path) or 'project root'}"


def make_tags(path, language):
    tags = set()
    parts = path.replace(os.sep, "/").split("/")
    for p in parts:
        if p and not p.startswith("."):
            tags.add(p.lower().replace("_", "-").replace(".", "-"))
    if language and language != "unknown":
        tags.add(language.lower())
    # Add some standard tags
    if path.endswith((".h", ".hpp")):
        tags.add("header")
    if path.endswith((".cpp", ".cc", ".c")):
        tags.add("source")
    if path.endswith(".md"):
        tags.add("documentation")
    if "makefile" in path.lower() or language == "makefile":
        tags.add("build")
    return list(tags)[:5]


def process_batch(batch_idx):
    with open(f"{TMP}/ua-file-analyzer-input-{batch_idx}.json") as f:
        inp = json.load(f)
    with open(f"{TMP}/ua-file-extract-results-{batch_idx}.json") as f:
        ext = json.load(f)

    batch_files = inp["batchFiles"]
    batch_imports = inp["batchImportData"]
    ext_results = ext["results"]

    nodes = []
    edges = []

    # Build lookup: path -> extraction result
    ext_by_path = {r["path"]: r for r in ext_results}

    # Also build dir-to-files mapping for non-code edges
    dir_files = defaultdict(list)
    for fm in batch_files:
        d = os.path.dirname(fm["path"])
        dir_files[d].append(fm)

    # ---- FILE NODES ----
    file_id_map = {}  # path -> node id
    for fm in batch_files:
        path = fm["path"]
        lang = fm.get("language", "unknown")
        lines = fm.get("sizeLines", 0)
        cat = fm.get("fileCategory", "")
        node_type, prefix = type_from_file(fm)
        node_id = f"{prefix}:{path}"
        file_id_map[path] = node_id

        tags = make_tags(path, lang)
        summ = summarize_file(path, lang)

        nodes.append({
            "id": node_id,
            "type": node_type,
            "name": os.path.basename(path),
            "filePath": path,
            "summary": summ,
            "tags": tags,
            "complexity": complexity(lines),
            "fileCategory": cat,
            "language": lang,
            "sizeLines": lines,
        })

    # ---- SUB-FILE NODES (functions, classes) ----
    # Also track for edges
    sub_nodes = {}  # node_id -> info
    func_node_map = {}  # (filePath, funcName) -> node_id

    for r in ext_results:
        path = r["path"]
        if path not in file_id_map:
            continue

        functions = r.get("functions", []) or []
        classes = r.get("classes", []) or []
        exports = r.get("exports", []) or []

        exported_names = {e["name"] for e in exports}

        for func in functions:
            name = func["name"]
            sl = func.get("startLine", 0)
            el = func.get("endLine", 0)
            func_lines = el - sl + 1 if el >= sl else 0

            is_exported = name in exported_names
            is_significant = is_exported or func_lines >= 10

            if not is_significant:
                continue

            node_id = f"function:{path}:{name}"
            sub_nodes[node_id] = {
                "type": "function",
                "filePath": path,
                "name": name,
                "lineRange": [sl, el],
            }
            func_node_map[(path, name)] = node_id

            nodes.append({
                "id": node_id,
                "type": "function",
                "name": name,
                "filePath": path,
                "summary": f"Function {name} in {os.path.basename(path)}",
                "tags": ["function", lang.lower() if lang != "unknown" else "code"],
                "complexity": complexity(func_lines),
                "lineRange": [sl, el],
                "language": lang,
            })

        for cls in classes:
            name = cls["name"]
            methods = cls.get("methods", []) or []
            if len(methods) < 2:
                continue
            sl = cls.get("startLine", 0)
            el = cls.get("endLine", 0)
            cls_lines = el - sl + 1 if el >= sl else 0

            node_id = f"class:{path}:{name}"
            sub_nodes[node_id] = {
                "type": "class",
                "filePath": path,
                "name": name,
                "lineRange": [sl, el],
            }
            func_node_map[(path, name)] = node_id

            nodes.append({
                "id": node_id,
                "type": "class",
                "name": name,
                "filePath": path,
                "summary": f"Class {name} in {os.path.basename(path)}",
                "tags": ["class", lang.lower() if lang != "unknown" else "code"],
                "complexity": complexity(cls_lines),
                "lineRange": [sl, el],
                "language": lang,
            })

    # ---- EDGES ----

    # 1. Import edges (MANDATORY)
    for file_path, imported_paths in batch_imports.items():
        if file_path not in file_id_map:
            continue
        source_id = file_id_map[file_path]
        for imp_path in imported_paths:
            if imp_path in file_id_map:
                target_id = file_id_map[imp_path]
            else:
                target_id = f"file:{imp_path}"
            edges.append({
                "source": source_id,
                "target": target_id,
                "type": "imports",
                "direction": "forward",
                "weight": 0.7,
            })

    # 2. Contains edges: file -> function/class
    for node_id, info in sub_nodes.items():
        fp = info["filePath"]
        if fp in file_id_map:
            edges.append({
                "source": file_id_map[fp],
                "target": node_id,
                "type": "contains",
                "direction": "forward",
                "weight": 1.0,
            })

    # 3. Export edges (in addition to contains)
    for r in ext_results:
        path = r["path"]
        if path not in file_id_map:
            continue
        exports = r.get("exports", []) or []
        file_id = file_id_map[path]
        for exp in exports:
            name = exp["name"]
            # Could be function or class
            f_node = func_node_map.get((path, name))
            if f_node:
                edges.append({
                    "source": file_id,
                    "target": f_node,
                    "type": "exports",
                    "direction": "forward",
                    "weight": 0.8,
                })

    # 4. Call edges (cross-file)
    # First build: for each function name, which file(s) it's in
    # Since function names might not be unique across files, we're more specific
    # Group by (callerFile, caller) and (calleeFile, callee)
    func_in_file = {}  # (filePath, name) -> exists
    for (fp, name), nid in func_node_map.items():
        func_in_file[(fp, name)] = nid

    for r in ext_results:
        path = r["path"]
        cg = r.get("callGraph", []) or []
        for call in cg:
            caller = call.get("caller", "")
            callee = call.get("callee", "")
            if not caller or not callee:
                continue

            # Clean callee name - sometimes includes arguments
            callee_clean = callee.split("(")[0].strip()
            if not callee_clean:
                continue

            caller_key = (path, caller)
            if caller_key not in func_in_file:
                continue

            # Find callee in another file within this batch
            for (cfp, cname), cnid in func_in_file.items():
                if cfp == path:
                    continue  # same file
                if cname == callee_clean:
                    edges.append({
                        "source": func_in_file[caller_key],
                        "target": cnid,
                        "type": "calls",
                        "direction": "forward",
                        "weight": 0.8,
                    })

    # 5. Non-code edges (configures, documents, deploys, related)
    # Configures: infra/config files -> code files in same dir
    for fm in batch_files:
        path = fm["path"]
        cat = fm.get("fileCategory", "")
        if cat not in ("infra", "config"):
            continue
        d = os.path.dirname(path)
        for other in dir_files.get(d, []):
            if other["path"] == path:
                continue
            if other.get("fileCategory") == "code" and other.get("language") in ("c++", "c"):
                edges.append({
                    "source": file_id_map[path],
                    "target": file_id_map[other["path"]],
                    "type": "configures",
                    "direction": "forward",
                    "weight": 0.6,
                })

    # Documents: docs -> code in same dir
    for fm in batch_files:
        path = fm["path"]
        cat = fm.get("fileCategory", "")
        if cat != "docs":
            continue
        d = os.path.dirname(path)
        for other in dir_files.get(d, []):
            if other["path"] == path:
                continue
            if other.get("fileCategory") == "code":
                edges.append({
                    "source": file_id_map[path],
                    "target": file_id_map[other["path"]],
                    "type": "documents",
                    "direction": "forward",
                    "weight": 0.5,
                })

    # Related: files in same dir with similar language/topic
    for d, files in dir_files.items():
        if len(files) < 2:
            continue
        for i in range(len(files)):
            for j in range(i + 1, len(files)):
                f1 = files[i]
                f2 = files[j]
                # Skip if already connected via other edges
                # Use if both are code files in same directory
                if f1.get("fileCategory") == "code" and f2.get("fileCategory") == "code":
                    edges.append({
                        "source": file_id_map[f1["path"]],
                        "target": file_id_map[f2["path"]],
                        "type": "related",
                        "direction": "undirected",
                        "weight": 0.5,
                    })

    # Deduplicate edges
    seen_edges = set()
    unique_edges = []
    for e in edges:
        key = (e["source"], e["target"], e["type"], e.get("direction", "forward"))
        if key not in seen_edges:
            seen_edges.add(key)
            unique_edges.append(e)

    output = {"nodes": nodes, "edges": unique_edges}
    out_path = f"{OUT}/batch-{batch_idx}.json"
    with open(out_path, "w") as f:
        json.dump(output, f, indent=2)

    return len(nodes), len(unique_edges)


if __name__ == "__main__":
    print("| Batch | Nodes | Edges |")
    print("|-------|-------|-------|")
    total_n = 0
    total_e = 0
    for i in range(49, 65):
        n, e = process_batch(i)
        total_n += n
        total_e += e
        print(f"| {i} | {n} | {e} |")
    print(f"| **Total** | **{total_n}** | **{total_e}** |")
    print()
    # Verify all files written
    for i in range(49, 65):
        p = f"{OUT}/batch-{i}.json"
        if os.path.exists(p):
            sz = os.path.getsize(p)
            print(f"verified: {p} ({sz} bytes)")
        else:
            print(f"MISSING: {p}")
