import random
import sys
import os
import struct

# Parse args
l = len(sys.argv)
if l < 4:
    print "Usage: python datagen.py <zipf> <card_d> <ratio_d_to_f> {<seed>}"
    quit()

zipf = float(sys.argv[1])
dim = int(sys.argv[2])
ratio_d_to_f = int(sys.argv[3])
fact = ratio_d_to_f*dim

if l > 4:
    seed = int(sys.argv[4])
else:
    seed = 0

# Download wiscer if not present
if not os.path.isdir("wiscer"):
    os.system("git clone https://github.com/aarati-K/wiscer.git")

# Generate the workload file
workload_str = "zipf={}" + '\n' + \
    "initialSize={}" + '\n' + \
    "operationCount={}" + '\n' + \
    "distShiftFreq={}" + '\n' + \
    "distShiftPrct=0" + '\n' + \
    "fetchProportion=1" + '\n' + \
    "insertProportion=0" + '\n' + \
    "deleteProportion=0" + '\n' + \
    "keyPattern=sequential" + '\n' + \
    "storageEngine=none" + '\n'
workload = workload_str.format(zipf, dim, fact, fact+1)
# print workload

# Store workload
f = open("wiscer/workloads/hashjoin", "w")
f.write(workload)
f.close()

# Run Wiscer
os.chdir("wiscer/")
os.system("make store")
os.system("./benchmark.out workloads/hashjoin {}".format(seed))
os.chdir("../")

# Parse the workload files generated in Wiscer
f = open("wiscer/keys", "rb")
content = f.read()
num_keys = len(content)/8
if num_keys != dim:
    print "Num keys do not match!"
keys = struct.unpack("l"*num_keys, content)
f.close()

dimfile = open('dim.tbl', 'w')
for i in keys:
    dimfile.write("{}|{}\n".format(i, i))
dimfile.close()

f = open("wiscer/ops", "rb")
factfile = open('fact.tbl', 'w')
content = f.read()
num_ops = len(content)/9
if num_ops != fact:
    print "Num ops do not match!"
for i in range(num_ops):
    op = struct.unpack('l', content[(9*i+1):(9*i+9)])[0]
    factfile.write("{}|{}\n".format(op, op))
f.close()
factfile.close()
