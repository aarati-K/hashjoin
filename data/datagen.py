import random
import sys

if sys.argv > 1:
    seed = int(sys.argv[1])
    random.seed(seed)

d = 1500000
f = 10*d

# Generate the dim column
dim = range(1, d+1)

for i in range(5):
    random.shuffle(dim)

prob = [0]*d
zipf = 2
for i in range(d):
    prob[i] = 1.0/pow(i+1, zipf)

cum = sum(prob)
for i in range(d):
    prob[i] = prob[i]/cum
    prob[i] = prob[i]*f

fact = [0]*f
i = 0
for j in range(d):
    for k in range(int(round(prob[j]))):
        fact[i] = dim[j]
        i += 1
        if (i == f):
            break
    if (i == f):
        break

j = 0;
while i < f:
    fact[i] = dim[j]
    i += 1
    j += 1

# for i in range(3):
#     random.shuffle(fact)
#     random.shuffle(dim)

dimfile = open('dim.tbl', 'w')
for i in dim:
    dimfile.write("{}|{}\n".format(i, i))
dimfile.close()

factfile = open('fact.tbl', 'w')
for i in fact:
    factfile.write("{}|{}\n".format(i, i))
factfile.close()
