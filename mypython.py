#C:Python31python.exe
# -*- coding:utf-8 -*-
#Jui-Hung Lu 977293709
#python3 mypython.py
import random
filename=["file1","file2","file3"]
word='qwertyuiopasdfghjklzxcvbnm'
for i in range(0,3):
    f = open(filename[i], 'w')
    for j in range(0,10):
        x = random.randint(0,25)
        f.write(word[x])
    f.write("\n")
for i in range(0,3):
    with open(filename[i], 'r') as f:
        for k in f:
            print(k,end="")
x = random.randint(1,42)
y = random.randint(1,42)
print(x)
print(y)
print(x*y)
