# compara metricile globale intre mai multe trace-uri (ex. EDF vs RM vs DM)
# rulare: python compare.py traces/trace_EDF.csv traces/trace_RM.csv traces/trace_DM.csv
import argparse
from pathlib import Path
import pandas as pd


POLICIES = {"edf", "rm", "dm", "priority"}


# policy-ul din numele fisierului; il caut printre token-ele cunoscute
def policy_name(path):
    parts = Path(path).stem.split("_")
    return next((p for p in reversed(parts) if p.lower() in POLICIES), parts[-1])


def summary(path):
    df = pd.read_csv(path)
    ticks = df[df.event == "tick"]
    n = len(ticks)
    busy = int(ticks["cpu_busy"].sum())
    # dispatches = de cate ori se incarca un task pe CPU = numarul real de context switch-uri.
    # preemptions = subsetul acelora care au dat la o parte un task inca activ (costul real).
    # nu tin o coloana "context_switches" agregata: ar fi dublat preemptiile (fiecare
    # preempt vine oricum cu un dispatch al taskului care intra).
    dispatches = int((df.event == "dispatch").sum())
    preemptions = int((df.event == "preempt").sum())
    return {
        "trace": Path(path).name,
        "policy": policy_name(path),
        "cpu_util_%": round(100 * busy / n, 2) if n else 0,
        "dispatches": dispatches,
        "preemptions": preemptions,
        "deadline_misses": int((df.event == "deadline_miss").sum()),
    }


ap = argparse.ArgumentParser()
ap.add_argument("traces", nargs="+")
args = ap.parse_args()

table = pd.DataFrame([summary(p) for p in args.traces])
try:
    print(table.to_markdown(index=False))
except ImportError:
    print(table.to_string(index=False))
