from tabulate import tabulate
import sys

if len(sys.argv) < 2:
    print "Usage: python2 parse_results.py filename"
    exit()

f = open(sys.argv[1])
median_time_default = []
median_time_vip = []
table_head = ["Skew level",
    "Median Time Default (s)",
    "Median Time VIP (s)",
    "Time VIP vs Default Hashjoin"]
workload_labels = []

readings_d = []
readings_a = []
flag = False
for line in f:
    if line[0] == 'D' or line[0] == 'R':
        print line.split('\n')[0]
        continue
    if line[0] == 'Z' or line[0] == '!':
        if line[0] == 'Z':
            workload_labels.append(line)
        l = len(readings_d)
        if not l:
            continue
        # Sort the readings by time
        readings_d = sorted(readings_d)
        med_time = (readings_d[l/2-1] + readings_d[l/2])/2
        median_time_default.append(med_time)
        readings_d = []

        readings_a = sorted(readings_a)
        med_time = (readings_a[l/2-1] + readings_a[l/2])/2
        median_time_vip.append(med_time)
        readings_a = []
    
    if line[0] == '!':
        # Generate the table
        table = []
        rows = []
        for i in range(len(median_time_default)):
            row = []
            row.append(workload_labels[i])
            row.append("{0:.2f}".format(median_time_default[i]/1000000.0))
            row.append("{0:.2f}".format(median_time_vip[i]/1000000.0))
            reduction = 100*(median_time_vip[i]/float(median_time_default[i]) - 1)
            row.append("{0:+.1f}%".format(reduction))
            table.append(row)
        print(tabulate(table, headers=table_head, tablefmt="grid"))
        # Reset metrics
        median_time_default = []
        median_time_vip = []
        workload_labels = []
        readings_d = []
        readings_a = []
        continue

    # Accumulate the readings
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
