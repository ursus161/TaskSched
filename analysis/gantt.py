# deseneaza un Gantt chart dintr-un trace CSV
# o banda per task unde ruleaza pe CPU, plus markere pt release / deadline miss
# rulare: python gantt.py traces/trace_EDF_xxx.csv [--out gantt.png]
import argparse
from pathlib import Path
import matplotlib.pyplot as plt
import pandas as pd


ap = argparse.ArgumentParser()
ap.add_argument("trace")
ap.add_argument("--out", default=None)  # daca lipseste, afiseaza fereastra
args = ap.parse_args()

df = pd.read_csv(args.trace)
# policy-ul din numele fisierului; il caut printre token-ele cunoscute
parts = Path(args.trace).stem.split("_")
policy = next((p for p in reversed(parts) if p.lower() in {"edf", "rm", "dm", "priority"}), parts[-1])

# taskurile in ordinea id-ului, fiecare pe cate o linie (lane)
ids = sorted(df[df.task_id >= 0]["task_id"].unique())
names = df[df.task_id >= 0].drop_duplicates("task_id").set_index("task_id")["task_name"].to_dict()
lane = {tid: i for i, tid in enumerate(ids)}

fig, ax = plt.subplots(figsize=(14, 0.5 * len(ids) + 2))

# din randurile de tick iau cine a rulat in fiecare tick -> bare de 1 tick
ticks = df[(df.event == "tick") & (df.running_task_id >= 0)]
for tid in ids:
    runs = ticks[ticks.running_task_id == tid]["tick"].tolist()
    bars = [(t, 1) for t in runs]  # broken_barh cere (start, latime)
    ax.broken_barh(bars, (lane[tid] - 0.4, 0.8), facecolors="tab:blue")

# markere: release (verde) si deadline miss (x rosu)
rel = df[df.event == "release"]
ax.scatter(rel["tick"], [lane[t] for t in rel["task_id"]], marker="|", color="green", s=120, label="release")
miss = df[df.event == "deadline_miss"]
if not miss.empty:
    ax.scatter(miss["tick"], [lane[t] for t in miss["task_id"]], marker="x", color="red", s=60, label="deadline miss")

ax.set_yticks(range(len(ids)))
ax.set_yticklabels([names[t] for t in ids])
ax.set_xlabel("tick")
ax.set_title(f"Gantt - {policy}")
ax.legend(loc="upper right")
ax.grid(axis="x", alpha=0.3)
plt.tight_layout()

if args.out:
    plt.savefig(args.out, dpi=120)
    print(f"scris in {args.out}")
else:
    plt.show()
