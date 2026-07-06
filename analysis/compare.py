# compara metricile globale intre mai multe trace-uri (ex. EDF vs RM vs DM)
# rulare: python compare.py traces/trace_EDF.csv traces/trace_RM.csv traces/trace_DM.csv
import argparse
import os
import pandas as pd


# policy-ul din numele fisierului: trace_<policy>_<timestamp>.csv
def policy_name(path):
    parts = os.path.basename(path).replace(".csv", "").split("_")
    return parts[1] if len(parts) > 1 else parts[0]


def summary(path):
    df = pd.read_csv(path)
    ticks = df[df.event == "tick"]
    n = len(ticks)
    busy = int(ticks["cpu_busy"].sum())
    ctx = int(((df.event == "dispatch") | (df.event == "preempt")).sum())
    return {
        "trace": os.path.basename(path),
        "policy": policy_name(path),
        "cpu_util_%": round(100 * busy / n, 2) if n else 0,
        "context_switches": ctx,
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
