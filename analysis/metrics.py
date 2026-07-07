# metrics per task + globale dintr-un trace CSV
# rulare: python metrics.py traces/trace_EDF_xxx.csv [--out metrics.csv]
import argparse
from pathlib import Path
import pandas as pd


POLICIES = {"edf", "rm", "dm", "priority"}


# policy-ul se vede din numele fisierului; il caut printre token-ele cunoscute
def policy_name(path):
    parts = Path(path).stem.split("_")
    return next((p for p in reversed(parts) if p.lower() in POLICIES), parts[-1])


def per_task(df):
    # numele fiecarui task id, ca sa le am in tabel
    names = df[df.task_id >= 0].drop_duplicates("task_id").set_index("task_id")["task_name"].to_dict()

    rows = []
    for tid, name in sorted(names.items()):
        releases = df[(df.task_id == tid) & (df.event == "release")]["tick"].tolist()
        completes = df[(df.task_id == tid) & (df.event == "complete")]["tick"].tolist()
        misses = ((df.task_id == tid) & (df.event == "deadline_miss")).sum()
        preempted = ((df.task_id == tid) & (df.event == "preempt")).sum()

        # response time = cat a durat de la release pana la complete, pentru fiecare job
        resp = []
        ri = 0
        for ct in completes:
            # gasesc release-ul cel mai recent dinainte de complete
            while ri + 1 < len(releases) and releases[ri + 1] <= ct:
                ri += 1
            if ri < len(releases) and releases[ri] <= ct:
                resp.append(ct + 1 - releases[ri])

        rows.append({
            "task_id": tid,
            "task_name": name,
            "jobs": len(completes),
            "resp_avg": round(sum(resp) / len(resp), 2) if resp else 0,
            "resp_max": max(resp) if resp else 0,
            "deadline_misses": int(misses),
            "times_preempted": int(preempted),
        })
    return pd.DataFrame(rows)


def globals_(df, policy):
    ticks = df[df.event == "tick"]
    n = len(ticks)
    busy = int(ticks["cpu_busy"].sum())
    # context switch = orice dispatch sau preempt
    ctx = int(((df.event == "dispatch") | (df.event == "preempt")).sum())
    return {
        "policy": policy,
        "ticks": n,
        "cpu_util_%": round(100 * busy / n, 2) if n else 0,
        "context_switches": ctx,
        "deadline_misses": int((df.event == "deadline_miss").sum()),
    }


def show(df):
    # to_markdown are nevoie de tabulate, altfel dau print simplu
    try:
        return df.to_markdown(index=False)
    except ImportError:
        return df.to_string(index=False)


ap = argparse.ArgumentParser()
ap.add_argument("trace")
ap.add_argument("--out", default="metrics.csv")
args = ap.parse_args()

df = pd.read_csv(args.trace)
pt = per_task(df)

print(f"# Metrics - {args.trace}\n")
print("## Global\n")
print(show(pd.DataFrame([globals_(df, policy_name(args.trace))])))
print("\n## Per task\n")
print(show(pt))

pt.to_csv(args.out, index=False)
print(f"\nscris in {args.out}")
