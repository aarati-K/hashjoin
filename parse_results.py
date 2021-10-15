from tabulate import tabulate

f = open('output.txt')
med_d = []
med_a = []
readings_d = []
readings_a = []
flag = False
for line in f:
    if line[0] == 'Z' or line[0] == '!':
        # Sort the readings by time
        l = len(readings_d)
        if not l:
            continue
        readings_d = sorted(readings_d)
        med_time = (readings_d[l/2-1] + readings_d[l/2])/2
        med_d.append(med_time)
        readings_d = []

        readings_a = sorted(readings_a)
        med_time = (readings_a[l/2-1] + readings_a[l/2])/2
        med_a.append(med_time)
        readings_a = []
        if line[0] == '!':
            break
        continue

    if line[0] != '|':
        continue
    readings = line.split(' ')[1:]
    readings = map(lambda x: int(x), readings)
    if not flag:
        readings_d.append(readings[0])
        flag = True
    else:
        readings_a.append(readings[0])
        flag = False
f.close()

head = ["Skew level",
    "Median Time Default (s)",
    "Median Time VIP (s)",
    "Time VIP vs Default Hashjoin"]

skews = ["Uniform (zipf=0)",
    "Low Skew (zipf=1)",
    "Medium Skew (zipf=2)",
    "High Skew (zipf=3)"]

table = []
for i in range(len(med_d)):
    row = []
    row.append(skews[i])
    row.append("{0:.2f}".format(med_d[i]/1000000.0))
    row.append("{0:.2f}".format(med_a[i]/1000000.0))
    reduction = 100*(med_a[i]/float(med_d[i]) - 1)
    row.append("{0:+.1f}%".format(reduction))
    table.append(row)

print(tabulate(table, headers=head, tablefmt="grid"))
