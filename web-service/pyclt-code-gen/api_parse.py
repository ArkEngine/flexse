# -----------------------------------------------------------------------------
# cparse.py
#
# Simple parser for ANSI C.  Based on the grammar in K&R, 2nd Ed.
# -----------------------------------------------------------------------------

import sys, copy
import api_lex
import ply.yacc as yacc
import ply.lex as lex

# Get the token map
tokens = api_lex.tokens

classs = {}
classs['methods'] = []
curparamlist=[]
curclassname = ""
curclasslist = []
curmethodname = ""
curmethodlist = []
curparamname = "__init__"
trace = 0
curtype = ""
curbalance = ""
curdefault = ""
mytrace = 0
debug = 0

# class
def p_all_class_specifier(t):
    'all_class_specifier : class_specifier_list'
    pass

def p_class_specifier(t):
    'class_specifier : CLASS ID LBRACE method_specifier_list RBRACE SEMI'
    global trace
    global curclassname, curclasslist, curmethodlist
    curclassname = str(t[2])
    trace += 1
    if mytrace:
        print trace, "classname: ", curclassname, "p_class_specifier"
    for x in curclasslist:
        if x['name'] == curclassname:
            print "repeat class name in one unit", "name:", curclassname
            sys.exit(1)
    classdict = {}
    classdict['name'] = curclassname
    classdict['methodlist'] = copy.copy(curmethodlist)
    curclasslist.append(copy.copy(classdict))
    curmethodlist = []
    curclassname = ""
    pass

def p_class_specifier_list_1(t):
    'class_specifier_list : class_specifier'
    global trace
    trace += 1
    if mytrace:
        print trace, "p_class_specifier_list_1"
    pass

def p_class_specifier_list_2(t):
    'class_specifier_list : class_specifier_list class_specifier'
    global trace
    trace += 1
    if mytrace:
        print trace, "p_class_specifier_list_2"
    pass

def p_method_specifier(t):
    'method_specifier : METHOD ID LPAREN param_specifier_list RPAREN SEMI'
    global trace, curmethodname, curparamlist
    curmethodname = str(t[2])
    trace += 1
    if mytrace:
        print trace, "methodname: ", curmethodname, "p_method_specifier"
    for x in curmethodlist:
        if x['name'] == curmethodname:
            print "repeat method name in one class", "name:", curmethodname
            sys.exit(1)
    methoddict = {}
    methoddict['name'] = curmethodname
    methoddict['paramlist'] = curparamlist
    curmethodlist.append(copy.copy(methoddict))
    curparamlist = []
    pass

def p_method_specifier_list_1(t):
    'method_specifier_list : method_specifier'
    global trace
    trace += 1
    if mytrace:
        print trace, "p_method_specifier_list_1"
    pass

def p_method_specifier_list_2(t):
    'method_specifier_list : method_specifier_list method_specifier'
    global trace
    trace += 1
    if mytrace:
        print trace, "p_method_specifier_list_2"
    pass

def p_type_specifier(t):
    '''type_specifier : UINT32_T
                      | INT32_T
                      | UINT64_T
                      | INT64_T
                      | BINARY
                      | STRING
                      | DOUBLE
                      | ID
                      '''
    global trace, curparamname, curtype
    trace += 1
    curtype = str(t[1])
    if mytrace:
        print trace, "type:", curtype
#                      | struct_specifier
    pass

def p_const_specifier(t):
    '''const_specifier : ICONST
                       | FCONST
                       | SCONST
                       | CCONST
                       '''
    global trace, curparamname, curparamlist, curdefault
    trace += 1
    curdefault = str(t[1])
    assert len(curdefault) > 0
    if mytrace:
        print trace, "param default:", curdefault, "name", curparamname
    pass


def p_param_specifier_1(t):
    'param_specifier : BALANCE param_specifier'
    global curparamname, curparamlist
    assert curparamlist[1]['name'] == curparamname
    curparamlist[1]['balance'] = 1
    if mytrace:
        print "balance one: ", curparamname, curparamlist
    pass

def p_param_specifier_2(t):
    'param_specifier : type_specifier ID LBRACKET const_specifier RBRACKET COMMA'
    global trace, curparamname, curtype, curdefault
    trace += 1
    curparamname = str(t[2])
    for x in curparamlist:
        if x['name'] == curparamname:
            print "repeat param name in one method", "name:", curparamname
            sys.exit(1)
    paramdict = {}
    paramdict['name'] = curparamname
    paramdict['type'] = 'list'
    paramdict['lsize'] = int(curdefault)
    curparamlist.append(paramdict)
    curtype = ""
    curdefault = ""
    if mytrace:
        print trace, "name", curparamlist[0]['name'], \
            "default", curdefault, "p_param_specifier_2", curparamlist[0]

def p_param_specifier_3(t):
    'param_specifier : type_specifier ID COMMA'

    global trace, curparamname, curtype, curdefault
    curparamname = str(t[2])
    trace += 1
    for x in curparamlist:
        if x['name'] == curparamname:
            print "repeat param name in one method", "name:", curparamname
            sys.exit(1)
    paramdict = {}
    paramdict['name'] = curparamname
    paramdict['type'] = curtype
    curparamlist.append(paramdict)
    curtype = ""
    if len(curdefault) > 0:
        for x in curparamlist:
            if x['name'] == curparamname:
                x['default'] = curdefault
    curdefault = ""
    if mytrace:
        print trace, "name", curparamlist[0]['name'], \
            "default", curdefault, "p_param_specifier_3", curparamlist[0]
    pass

def p_param_specifier_4(t):
#    'param_specifier : type_specifier ID EQUALS DEFAULT LPAREN const_specifier RPAREN COMMA'
    'param_specifier : type_specifier ID EQUALS DEFAULT LPAREN const_specifier RPAREN COMMA'
    global trace, curparamname, curtype, curdefault
    curparamname = str(t[2])
    trace += 1
    for x in curparamlist:
        if x['name'] == curparamname:
            print "repeat param name in one method", "name:", curparamname
            sys.exit(1)
    paramdict = {}
    paramdict['name'] = curparamname
    paramdict['type'] = curtype
    curparamlist.append(paramdict)
    curtype = ""
    if len(curdefault) > 0:
        for x in curparamlist:
            if x['name'] == curparamname:
                x['default'] = curdefault
    curdefault = ""
    if mytrace:
        print trace, "name", curparamlist[0]['name'], \
            "default", curdefault, "p_param_specifier_4", curparamlist[0],
    pass

def p_param_specifier_5(t):
    'param_specifier : type_specifier ID EEQUALS DEFAULT LPAREN const_specifier RPAREN COMMA'
    global trace, curparamname, curtype, curdefault
    curparamname = str(t[2])
    for x in curparamlist:
        if x['name'] == curparamname:
            print "repeat param name in one method", "name:", curparamname
            sys.exit(1)
    paramdict = {}
    paramdict['name'] = curparamname
    paramdict['type'] = curtype
    curparamlist.append(paramdict)
    curtype = ""
    if len(curdefault) > 0:
        for x in curparamlist:
            if x['name'] == curparamname:
                x['default'] = curdefault
                x['hide'] = 1
    curdefault = ""
    if mytrace:
        print trace, "name", curparamlist[0]['name'], \
            "hide default", curdefault, "p_param_specifier_4", curparamlist[0],
    pass


def p_param_specifier_list_1(t):
    'param_specifier_list : param_specifier'
    pass

def p_param_specifier_list_2(t):
    'param_specifier_list : param_specifier_list param_specifier'
    pass

def p_error(t):
    print("Whoa. We're hosed")
    assert 0

def isint(string):
    try:
        int(string)
    except ValueError:
        return False
    return True

def check(classdict):
    for c in classdict:
#        print "cccccccccccc", c
        iclassname = c['name']
        iclassdesc = c['methodlist']
        if debug:
            print "------------------------------------------"
            print "class:", iclassname
        assert iclassdesc != []
        for m in iclassdesc:
            imethodname = m['name']
            imethoddesc = m['paramlist']
            if debug:
                print "  method:", imethodname
            hasbalance = 0
            for p in imethoddesc:
#                print "pppppppppppp", p
                iparamname = p['name']
                iparamdesc = p
                curbalance = iparamdesc.has_key('balance')
                if  hasbalance and curbalance :
                    print "method", imethodname, "has two balance key, invalid"
                    sys.exit(1)
                if curbalance:
                    curbalance = 1
                    hasbalance = 1
                    if iparamdesc['type'] == 'list':
                        print "param", iparamname, "is a list, can't be set to balance, stupid"
                        sys.exit(1)
                else:
                    curbalance = 0
                # type check
                if debug:
                    print "   ", iparamname, "-", "type:", iparamdesc['type'],
                    if debug:
                        if iparamdesc['type'] == 'list':
                            print "[", iparamdesc['lsize'], "]",
                if debug:
                    print "balance", curbalance,
                if iparamdesc.has_key('default') and iparamdesc.has_key('balance'):
                    print "param", iparamname, "has default value, can't be set to balance, stupid"
                    sys.exit(1)
                    pass
                if iparamdesc.has_key('default'):
                    if  iparamdesc['type'] == 'uint32_t'         \
                        or  iparamdesc['type'] == 'int32_t'  \
                        or  iparamdesc['type'] == 'uint64_t' \
                        or  iparamdesc['type'] == 'int64_t':
                        if not isint(iparamdesc['default']):
                            print "param:", iparamname, "type:", iparamdesc['type'], \
                                "default:", iparamdesc['default'], "value type error."
                            sys.exit(1)
                    if debug:
                        print "default:", iparamdesc['default'],
                    else:
                        pass
                if debug:
                    print

def parse(filename):
    f = file(filename)
    input=""
    while True:
        line = f.readline()
        if len(line) == 0:
            break
        input += line

    yacc.yacc(method='LALR')
    yacc.parse(input, debug = 0)
    check(curclasslist)
    return curclasslist
