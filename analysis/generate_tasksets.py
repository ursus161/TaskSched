"""Generator de task set-uri sintetice prin UUniFast (Bini & Buttazzo, 2005).

UUniFast imparte o utilizare totala U in n utilizari u_i (suma = U) nebiasat:
fiecare combinatie valida e la fel de probabila. Normalizarea naiva ar favoriza
seturi cu utilizari egale, deci ai masura schedulabilitatea pe un colt ingust din
spatiul de intrari in loc de seturi reprezentative.

Per task: perioada T log-uniforma, WCET = round(u_i * T), deadline = T (implicit)
sau, cu --constrained, in [WCET, T].

    python generate_tasksets.py --n 5 --util 0.7 --count 20 --out-dir configs/generated
"""
import argparse
import math
import random
from pathlib import Path


# UUniFast: n utilizari care insumeaza U, distributie nebiasata
def uunifast(n, u_total, rng):
    utils = []
    s = u_total
    for i in range(1, n):
        s_next = s * rng.random() ** (1.0 / (n - i))
        utils.append(s - s_next)
        s = s_next
    utils.append(s)
    return utils


def make_taskset(n, u_total, pmin, pmax, constrained, rng):
    rows = []
    for i, u in enumerate(uunifast(n, u_total, rng), start=1):
        # perioada log-uniforma pe [pmin, pmax]
        period = round(math.exp(rng.uniform(math.log(pmin), math.log(pmax))))
        wcet = max(1, round(u * period))
        deadline = rng.randint(wcet, period) if constrained and wcet < period else period
        # priority folosita doar de policy-ul Priority; dau valori distincte
        rows.append(f"periodic,T{i},{period},{wcet},{deadline},{i},0")
    return rows


ap = argparse.ArgumentParser(description="Genereaza task set-uri sintetice prin UUniFast.")
ap.add_argument("--n", type=int, required=True, help="taskuri per set")
ap.add_argument("--util", type=float, required=True, help="utilizare totala tinta (ex. 0.7)")
ap.add_argument("--count", type=int, default=1, help="cate seturi sa generez")
ap.add_argument("--out-dir", default="configs/generated", help="directorul de output")
ap.add_argument("--pmin", type=int, default=10, help="perioada minima")
ap.add_argument("--pmax", type=int, default=1000, help="perioada maxima")
ap.add_argument("--constrained", action="store_true", help="deadline in [WCET, T] in loc de D=T")
ap.add_argument("--seed", type=int, help="seed pentru reproducibilitate")
args = ap.parse_args()

rng = random.Random(args.seed)  
out_dir = Path(args.out_dir)
out_dir.mkdir(parents=True, exist_ok=True)

header = "type,name,period,wcet,deadline,priority,release_time"
tag = f"u{round(args.util * 100):03d}"  # 0.7 -> u070

for k in range(args.count):
    rows = make_taskset(args.n, args.util, args.pmin, args.pmax, args.constrained, rng)
    path = out_dir / f"gen_{tag}_{k:02d}.csv"
    path.write_text("\n".join([header, *rows]) + "\n")

print(f"{args.count} seturi scrise in {out_dir}/ (gen_{tag}_*.csv)")
