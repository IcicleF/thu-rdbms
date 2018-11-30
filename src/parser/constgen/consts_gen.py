# coding: utf-8

import codecs

keywds = codecs.open('consts.txt', encoding='utf-8').readlines()
ouf = codecs.open('constsgen.txt', 'w', encoding='utf-8')

l = len(keywds)
begin = 1000
for i in range(l):
    kwd = keywds[i][:-1]
    if i == 0:
        ouf.write('const int ' + kwd + ' = ' + str(begin) + ';\n')
    else:
        fwd = keywds[i - 1][:-1]
        ouf.write('const int ' + kwd + ' = ' + fwd + ' + 1;\n')
ouf.close()