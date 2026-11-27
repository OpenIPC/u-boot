#!/usr/bin/env python3
"""Pick the legs of a build-<vendor>.yml run from what the change touches.

The matrices are fixed lists (hisilicon: 39 build legs and 31 QEMU smoke legs),
and every leg queues for a hosted runner of its own. A run spends most of its
wall clock waiting for those runners, not building: hisilicon run 45887645050
was 74 jobs, 68 job-minutes and 135 minutes long. So a change that cannot
reach a leg should not queue it.

The rules are one-directional, as in widgetii/majestic scripts/ci-matrix.py
and OpenIPC/firmware .github/scripts/ci-matrix.py: a path has to be
*recognised* before it can narrow anything. A path nothing claims (common/,
drivers/, arch/, build.sh, openipc-common.h, ...) widens to every leg. Getting
the classification wrong therefore costs runner time, never coverage.

    docs only                      -> nothing to build
    paths owned by known legs      -> those legs (see `owners` in .github/ci/)
    anything else                  -> every leg

What a run covers is decided by the event:

    workflow_dispatch              -> every leg (it releases and publishes)
    pull_request into `openipc`    -> every leg (the workflow itself changed)
    pull_request into the vendor   -> its merge commit against its base
    push to the vendor branch      -> nothing, when the PR that produced this
                                      tree already built it (refs/ci/built/);
                                      else the pushed range

The `ci:full` label on a pull request forces every leg.

Usage:
    ci-select.py VENDOR --src DIR            # in Actions: writes $GITHUB_OUTPUT
    ci-select.py VENDOR --src DIR --stdin    # a file list on stdin, prints it
    ci-select.py VENDOR --src DIR --self-test
"""

import argparse
import fnmatch
import json
import os
import re
import subprocess
import sys

import yaml

HERE = os.path.dirname(os.path.abspath(__file__))
CI_DIR = os.path.join(os.path.dirname(HERE), "ci")

FULL_LABEL = "ci:full"
ZERO_SHA = "0" * 40

# Never read by build.sh or the U-Boot build. Anchored so that only whole
# names match: a prefix rule would swallow real code, and skipping CI for
# real code is the one direction this must never fail in.
#   - documentation: doc/, Markdown anywhere, README files
#   - the CI caller and other GitHub metadata (.github/ on a vendor branch is
#     only the ci.yml caller; this run reaching the selector at all is the
#     proof that it still calls the workflow)
#   - openipc/tools/: host tools for working on the port (register dumps,
#     logo generators); no build.sh or Makefile on any openipc-* branch
#     references it
NO_BUILD = re.compile(
    r"^(?:doc/"
    r"|.*\.md$"
    r"|(?:.*/)?README(?:\.[^/]*)?$"
    r"|\.github/"
    r"|\.(?:gitignore|mailmap|checkpatch\.conf)$"
    r"|openipc/tools/)"
)

INCLUDE = re.compile(r'^\s*#\s*include\s*[<"](?:configs/)?([\w.+-]+\.h)[>"]', re.M)


def leg_key(entry):
    """The name a leg goes by. novatek builds `target` (nt98560-nand) from
    `soc` (nt98560), and its smoke legs name the target."""
    return entry.get("target", entry["soc"])


def legs_of(matrix):
    if "include" in matrix:
        return [leg_key(e) for e in matrix["include"]]
    return list(matrix["soc"])


def filter_matrix(matrix, keep):
    if "include" in matrix:
        rows = [e for e in matrix["include"] if leg_key(e) in keep]
        return ({**{k: v for k, v in matrix.items() if k != "include"},
                 "include": rows}, bool(rows))
    socs = [s for s in matrix["soc"] if s in keep]
    return {**matrix, "soc": socs}, bool(socs)


def load(vendor):
    with open(os.path.join(CI_DIR, f"{vendor}.yml")) as handle:
        return yaml.safe_load(handle)


def header_closure(src, name):
    """include/configs/<name> and the include/configs headers it includes."""
    seen, todo = set(), [name]
    while todo:
        header = todo.pop()
        if header in seen:
            continue
        path = os.path.join(src, "include/configs", header)
        if not os.path.isfile(path):
            continue
        seen.add(header)
        with open(path, errors="replace") as handle:
            todo += INCLUDE.findall(handle.read())
    return seen


def ownership(cfg, src):
    """{path or directory prefix: {legs}} from the `owners` table.

    A leg owns its own paths, its board directory, and its board header with
    every include/configs header that header pulls in. Several legs can own
    one path (the four hi3520d legs share hi3520dv100.h); a change there
    selects all of them.
    """
    owned = {}
    for leg, spec in (cfg.get("owners") or {}).items():
        claims = list(spec.get("paths", []))
        if spec.get("board"):
            claims.append(f"board/{spec['board']}/")
        if spec.get("config"):
            claims += [f"include/configs/{h}"
                       for h in header_closure(src, spec["config"] + ".h")]
        for claim in claims:
            owned.setdefault(claim, set()).add(leg)
    return owned


def owners_of(path, owned):
    legs = set()
    for claim, who in owned.items():
        if claim.endswith("/"):
            hit = path.startswith(claim)
        elif any(c in claim for c in "*?["):
            hit = fnmatch.fnmatchcase(path, claim)
        else:
            hit = path == claim
        if hit:
            legs |= who
    return legs


def decide(cfg, src, changed, reason_if_full=None):
    """Map changed paths to the set of legs (None = every leg)."""
    if reason_if_full:
        return None, reason_if_full
    if changed is None:
        return None, "could not tell what changed"
    owned = ownership(cfg, src)
    legs, docs = set(), 0
    for path in changed:
        if NO_BUILD.match(path):
            docs += 1
            continue
        who = owners_of(path, owned)
        if not who:
            return None, f"{path} reaches every leg"
        legs |= who
    if legs:
        return legs, "only " + " ".join(sorted(legs))
    if docs:
        return set(), "documentation and CI caller only"
    return set(), "no files changed"


def output(cfg, legs, reason, extra=None):
    build_all = legs_of(cfg["build"])
    keep = set(build_all) if legs is None else legs
    build, needs_build = filter_matrix(cfg["build"], keep)
    if "smoke" in cfg:
        smoke, needs_smoke = filter_matrix(cfg["smoke"], keep)
    else:
        smoke, needs_smoke = {"soc": []}, False
    needs_smoke = needs_smoke and needs_build
    # Legs smoke-tested by a job of their own rather than the matrix
    # (axera's ax630c-emmc): the shared qemu job has to run for them too.
    needs_qemu = needs_smoke or (needs_build and
                                 any(l in keep for l in cfg.get("smoke_extra", [])))
    full = legs is None or keep >= set(build_all)
    selected = [l for l in build_all if l in keep]
    result = {
        "build_matrix": json.dumps(build, separators=(",", ":")),
        "smoke_matrix": json.dumps(smoke, separators=(",", ":")),
        "needs_build": str(needs_build).lower(),
        "needs_smoke": str(needs_smoke).lower(),
        "needs_qemu": str(needs_qemu).lower(),
        "full": str(full).lower(),
        "legs": json.dumps(selected),
        "reason": reason,
    }
    result.update(extra or {})
    return result


def git(src, *args, check=True, timeout=None):
    """Run git in the source tree. Calls that reach the network get a
    timeout: a stuck remote must cost the run its narrowing, never hang it.
    A timeout reads as a failure, and every caller fails open on one."""
    try:
        return subprocess.run(["git", "-C", src, *args], check=check,
                              capture_output=True, text=True, timeout=timeout)
    except subprocess.TimeoutExpired:
        return subprocess.CompletedProcess(args, 124, "", f"timed out after {timeout}s")


def diff_names(src, base, head):
    """Changed paths base..head, old names of renames included: a rename's
    old path may be the only thing tying it to a leg."""
    proc = git(src, "diff", "--name-status", "--no-renames", base, head, check=False)
    if proc.returncode:
        print(f"ci-select: git diff {base} {head}: {proc.stderr.strip()}",
              file=sys.stderr)
        return None
    return [line.split("\t", 1)[1] for line in proc.stdout.splitlines() if "\t" in line]


def tree_of(src, rev):
    proc = git(src, "rev-parse", f"{rev}^{{tree}}", check=False)
    return proc.stdout.strip() if proc.returncode == 0 else None


def built_ref(vendor, tree, base_tree=None):
    """The marker a green pull request run leaves for the push that lands it.

    refs/ci/built/<vendor>/<tree>: every leg built and smoke-tested this tree.
    refs/ci/built/<vendor>/<tree>-from-<base tree>: the legs this change
    reaches were built on this tree, which is what the push of the same
    change onto the same base would select again.
    """
    name = f"refs/ci/built/{vendor}/{tree}"
    return f"{name}-from-{base_tree}" if base_tree else name


def remote_has(src, ref):
    proc = git(src, "ls-remote", "--exit-code", "origin", ref, check=False, timeout=60)
    if proc.returncode not in (0, 2):
        print(f"ci-select: ls-remote {ref}: {proc.stderr.strip()}", file=sys.stderr)
    return proc.returncode == 0


def fetch(src, sha):
    proc = git(src, "fetch", "--no-tags", "--depth=1", "origin", sha, check=False,
               timeout=120)
    if proc.returncode:
        print(f"ci-select: fetch {sha}: {proc.stderr.strip()}", file=sys.stderr)
    return proc.returncode == 0


def from_actions(vendor, cfg, src):
    event_name = os.environ.get("GITHUB_EVENT_NAME", "")
    path = os.environ.get("GITHUB_EVENT_PATH")
    event = {}
    if path and os.path.exists(path):
        with open(path) as handle:
            event = json.load(handle)
    branch = f"openipc-{vendor}"

    if event_name == "pull_request":
        pr = event.get("pull_request", {})
        base = pr.get("base", {}).get("ref")
        if base != branch:
            # A change to the workflow, this script or the matrices: the
            # self-test is the gate on the tables, and fails the run.
            if not self_test(vendor, cfg, src):
                sys.exit(1)
            return output(cfg, *decide(cfg, src, None,
                                       f"pull request into {base} builds every leg"))

    # On the vendor branch the tables are another branch's, so a tree they no
    # longer describe cannot be fixed in this change: build every leg (the
    # safe reading) and say why, rather than fail an unrelated pull request.
    if event_name in ("pull_request", "push") and not self_test(vendor, cfg, src):
        print("::warning::ci-select: .github/ci/%s.yml no longer describes this "
              "tree (see above); building every leg" % vendor)
        return output(cfg, *decide(cfg, src, None, "owners table out of date"))

    if event_name == "pull_request":
        # The checkout is the merge commit (github.sha), fetched two deep:
        # its first parent is the base it was merged onto.
        if FULL_LABEL in [l.get("name") for l in pr.get("labels", [])]:
            legs, reason = decide(cfg, src, None, f"{FULL_LABEL} label set")
        else:
            legs, reason = decide(cfg, src, diff_names(src, "HEAD^1", "HEAD"))
        tree, base_tree = tree_of(src, "HEAD"), tree_of(src, "HEAD^1")
        marker = ""
        if tree and base_tree:
            marker = built_ref(vendor, tree, None if legs is None else base_tree)
        return output(cfg, legs, reason, {"marker": marker})

    if event_name == "push":
        before, after = event.get("before", ""), event.get("after", "")
        tree = tree_of(src, "HEAD")
        if tree and remote_has(src, built_ref(vendor, tree)):
            return output(cfg, set(), f"tree {tree} already built in full "
                                       f"by its pull request")
        if not before or before == ZERO_SHA or not fetch(src, before):
            return output(cfg, *decide(cfg, src, None,
                                       "no earlier commit to compare against"))
        before_tree = tree_of(src, before)
        if tree and before_tree and remote_has(src, built_ref(vendor, tree, before_tree)):
            return output(cfg, set(), f"tree {tree} already built from "
                                       f"{before_tree} by its pull request")
        return output(cfg, *decide(cfg, src, diff_names(src, before, after or "HEAD")))

    return output(cfg, *decide(cfg, src, None, f"{event_name or 'this event'} "
                                                f"builds every leg"))


def self_test(vendor, cfg, src):
    errors = []
    build = legs_of(cfg["build"])
    if len(build) != len(set(build)):
        errors.append("build legs are not unique")
    if "smoke" in cfg:
        stray = sorted(set(legs_of(cfg["smoke"])) - set(build))
        if stray:
            errors.append(f"smoke legs with no build leg: {stray}")
    stray = sorted(set(cfg.get("smoke_extra", [])) - set(build))
    if stray:
        errors.append(f"smoke_extra legs with no build leg: {stray}")
    for leg, spec in (cfg.get("owners") or {}).items():
        if leg not in build:
            errors.append(f"owners: {leg} is not a build leg")
        for claim in spec.get("paths", []):
            if any(c in claim for c in "*?["):
                continue
            if not os.path.exists(os.path.join(src, claim)):
                errors.append(f"owners: {leg}: {claim} does not exist")
        if spec.get("board") and not os.path.isdir(os.path.join(src, "board", spec["board"])):
            errors.append(f"owners: {leg}: board/{spec['board']} does not exist")
        if spec.get("config") and not os.path.isfile(
                os.path.join(src, "include/configs", spec["config"] + ".h")):
            errors.append(f"owners: {leg}: include/configs/{spec['config']}.h does not exist")
        # A fragment that names a board header or board directory that exists
        # must name one this leg claims: otherwise the leg moved to another
        # board, and a change to the new board would narrow past it. The
        # declared values are what `make olddefconfig` resolves, which is not
        # always what the fragment says: Kconfig ignores a value for a symbol
        # with no prompt (hi3516ev100_config says SYS_BOARD="hi3516ev100",
        # the build uses hi3516cv300; there is no board/hisilicon/hi3516ev100
        # to build). A value naming nothing that exists cannot be built and is
        # not checked.
        vendor_dir = (spec.get("board") or "/").split("/")[0]
        for claim in spec.get("paths", []):
            path = os.path.join(src, claim)
            if not claim.endswith("_config") or not os.path.isfile(path):
                continue
            with open(path, errors="replace") as handle:
                text = handle.read()

            def last(key):
                found = re.findall(rf'^CONFIG_{key}="([^"]*)"', text, re.M)
                return found[-1] if found else None

            name = last("SYS_CONFIG_NAME")
            if (name and name != spec.get("config") and
                    os.path.isfile(os.path.join(src, "include/configs", name + ".h"))):
                errors.append(f"owners: {leg}: {claim} names include/configs/{name}.h, "
                              f"owners say {spec.get('config')}")
            board = last("SYS_BOARD")
            if board:
                board = f"{last('SYS_VENDOR') or vendor_dir}/{board}"
                if (board != spec.get("board") and
                        os.path.isdir(os.path.join(src, "board", board))):
                    errors.append(f"owners: {leg}: {claim} names board/{board}, "
                                  f"owners say {spec.get('board')}")
    for error in errors:
        print(f"ci-select self-test ({vendor}): {error}", file=sys.stderr)
    if not errors:
        print(f"ci-select self-test ({vendor}): ok, {len(build)} build legs, "
              f"{len(cfg.get('owners') or {})} with owned paths")
    return not errors


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("vendor")
    parser.add_argument("--src", default=".", help="the checked-out vendor tree")
    parser.add_argument("--stdin", action="store_true",
                        help="classify a file list read from stdin and print it")
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()
    cfg = load(args.vendor)

    if args.self_test:
        sys.exit(0 if self_test(args.vendor, cfg, args.src) else 1)

    if args.stdin:
        changed = [line.strip() for line in sys.stdin if line.strip()]
        result = output(cfg, *decide(cfg, args.src, changed))
        print(f"{result['reason']}\n  build: {result['legs']}\n"
              f"  smoke: {result['needs_smoke']}")
        return

    result = from_actions(args.vendor, cfg, args.src)
    print(f"ci-select: {result['reason']}")
    print(f"ci-select: legs {result['legs']}")
    out = os.environ.get("GITHUB_OUTPUT")
    with open(out, "a") if out else sys.stdout as handle:
        for key, value in result.items():
            handle.write(f"{key}={value}\n")


if __name__ == "__main__":
    main()
