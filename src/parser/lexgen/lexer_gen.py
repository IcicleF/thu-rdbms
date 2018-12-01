# coding: utf-8

import codecs

keywds = codecs.open('keyword.txt', encoding='utf-8').readlines()
ouf = codecs.open('lexgen.txt', 'w', encoding='utf-8')

cur = 0
for kwd in keywds:
    kwd = kwd[:-1]
    if len(kwd) == 0:
        cur = 1
        ouf.write('\n')
    else:
        if cur == 0:
            ouf.write('"' + kwd + '"' + ' ' * (14 - len(kwd)) + '{ yylval = new AstKeyword(' + kwd + ', "' + kwd + '"); return ' + kwd + '; }\n')
        else:
            ouf.write('"' + kwd + '"' + ' ' * (14 - len(kwd)) + '{ yylval = new AstOper(\'' + kwd + '\'); return \'' + kwd + '\'; }\n')
ouf.close()