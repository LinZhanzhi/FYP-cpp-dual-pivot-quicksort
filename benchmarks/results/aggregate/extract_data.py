import pandas as pd
df = pd.read_csv('summary_representative.csv')

print("=== PARALLEL SCALING (int, RANDOM, 10M) ===")
seq = df[(df['Algorithm']=='dual_pivot_sequential') & (df['Pattern']=='RANDOM') & (df['Size']==10000000) & (df['Type']=='int')]['Time(ms)'].values[0]
print(f"1T (sequential): {seq:.2f} ms")

for t in [2, 4, 8, 16]:
    d = df[(df['Algorithm']==f'dual_pivot_parallel_{t}') & (df['Pattern']=='RANDOM') & (df['Size']==10000000) & (df['Type']=='int')]['Time(ms)'].values[0]
    speedup = seq / d
    eff = (speedup / t) * 100
    print(f"{t}T: {d:.2f} ms, speedup={speedup:.2f}x, efficiency={eff:.0f}%")

print("\n=== SEQUENTIAL COMPARISON (int, 10M) ===")
patterns = ['RANDOM', 'REVERSE_SORTED', 'ORGAN_PIPE', 'SAWTOOTH', 'NEARLY_SORTED', 'MANY_DUPLICATES_10', 'MANY_DUPLICATES_50', 'MANY_DUPLICATES_90']
for p in patterns:
    std = df[(df['Algorithm']=='std_sort') & (df['Pattern']==p) & (df['Size']==10000000) & (df['Type']=='int')]['Time(ms)'].values[0]
    dpqs = df[(df['Algorithm']=='dual_pivot_sequential') & (df['Pattern']==p) & (df['Size']==10000000) & (df['Type']=='int')]['Time(ms)'].values[0]
    speedup = std / dpqs
    print(f"{p}: std={std:.2f}ms, dpqs={dpqs:.2f}ms, speedup={speedup:.2f}x")

print("\n=== SMALL INTEGERS (1M and 10M) ===")
for typ in ['int8_t', 'int16_t']:
    for size in [1000000, 10000000]:
        std = df[(df['Algorithm']=='std_sort') & (df['Pattern']=='RANDOM') & (df['Size']==size) & (df['Type']==typ)]['Time(ms)'].values[0]
        dpqs = df[(df['Algorithm']=='dual_pivot_sequential') & (df['Pattern']=='RANDOM') & (df['Size']==size) & (df['Type']==typ)]['Time(ms)'].values[0]
        if dpqs > 0:
            speedup = std / dpqs
            print(f"{typ} {size//1000000}M: std={std:.2f}ms, dpqs={dpqs:.2f}ms, speedup={speedup:.2f}x")

print("\n=== SEQUENTIAL AT 100K, 1M, 10M ===")
for size in [100000, 1000000, 10000000]:
    for p in ['RANDOM', 'REVERSE_SORTED', 'ORGAN_PIPE', 'SAWTOOTH', 'NEARLY_SORTED']:
        std = df[(df['Algorithm']=='std_sort') & (df['Pattern']==p) & (df['Size']==size) & (df['Type']=='int')]['Time(ms)'].values[0]
        dpqs = df[(df['Algorithm']=='dual_pivot_sequential') & (df['Pattern']==p) & (df['Size']==size) & (df['Type']=='int')]['Time(ms)'].values[0]
        speedup = std / dpqs
        print(f"{size} {p}: std={std:.2f}ms, dpqs={dpqs:.2f}ms, speedup={speedup:.2f}x")
