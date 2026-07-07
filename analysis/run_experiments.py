# ruleaza toate policy-urile peste toate task set-urile din configs/
# doar stdlib + subprocess, fara dependinte
# rulare: python run_experiments.py [--policies edf rm] [--configs "configs/*.csv"] [--duration N]
import argparse
import subprocess
from pathlib import Path

ALL_POLICIES = ["edf", "rm", "dm", "priority"]

ap = argparse.ArgumentParser()
ap.add_argument("--policies", nargs="+", default=ALL_POLICIES, help="ce policy-uri sa rulez")
ap.add_argument("--configs", nargs="+", default=["configs/*.csv"], help="fisiere sau glob-uri de task set")
ap.add_argument("--duration", type=int, help="numar de tickuri (altfel default-ul din simulator)")
ap.add_argument("--bin", default="./tasksched", help="calea catre binar")
args = ap.parse_args()

# expandez glob-urile in lista de fisiere
configs = sorted(p for pat in args.configs for p in Path().glob(pat))
if not configs:
    raise SystemExit(f"run_experiments: niciun task set gasit pentru {args.configs}")

Path("traces").mkdir(exist_ok=True)

done, failed = 0, 0
for cfg in configs:
    for pol in args.policies:
        out = f"traces/{cfg.stem}_{pol}.csv"
        cmd = [args.bin, f"--policy={pol}", f"--taskset={cfg}", f"--out={out}"]
        if args.duration is not None:
            cmd.append(f"--duration={args.duration}")

        res = subprocess.run(cmd, capture_output=True, text=True)
        if res.returncode == 0:
            print(f"[ok]   {pol:8} x {cfg} -> {out}")
            done += 1
        else:
            print(f"[FAIL] {pol:8} x {cfg}: {res.stderr.strip() or res.stdout.strip()}")
            failed += 1

print(f"\n{done} trace-uri scrise in traces/, {failed} esuate")
