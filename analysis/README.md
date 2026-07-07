# analysis

Scripturi Python pentru task set-uri si trace-uri. Nu se compileaza cu proiectul C++.

`generate_tasksets.py` si `run_experiments.py` folosesc doar stdlib.
`metrics.py`, `compare.py`, `gantt.py` au nevoie de: `pip install -r requirements.txt`.

## Workflow complet

```bash
# 1. genereaza 20 de task set-uri sintetice (UUniFast, U=0.7, 5 taskuri)
python analysis/generate_tasksets.py --n 5 --util 0.7 --count 20 --out-dir configs/generated

# 2. ruleaza toate policy-urile peste toate seturile -> traces/<set>_<policy>.csv
python analysis/run_experiments.py --configs "configs/generated/*.csv"

# 3. compara global un set intre policy-uri
python analysis/compare.py traces/gen_u070_00_*.csv

# 4. Gantt pe un trace
python analysis/gantt.py traces/gen_u070_00_edf.csv --out gantt.png
```

## Scripturi

- **generate_tasksets.py** — task set-uri sintetice cu utilizare tinta (UUniFast). `--constrained` pt D<T, `--seed` pt reproducibilitate.
- **run_experiments.py** — ruleaza policy × task set peste `configs/`. `--policies edf rm` si `--configs "..."` restrang, `--duration N` se paseaza la simulator.
- **metrics.py** — metrici per task + globale dintr-un trace, scrie `metrics.csv`.
- **compare.py** — metrici globale pentru mai multe trace-uri, una langa alta.
- **gantt.py** — Gantt chart dintr-un trace.
