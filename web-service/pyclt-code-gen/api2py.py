# -----------------------------------------------------------------------------
# cparse.py
#
# Simple parser for ANSI C.  Based on the grammar in K&R, 2nd Ed.
# -----------------------------------------------------------------------------

#
# TODO
# (1) list sub types
# (2) int-bitfield
#
import sys, os
import api_parse

class CodeGen:
    __code = ""
    __step = ""
    __outfile = ""
    def __init__(self):
        self.__code = ""
        self.__step = " "*4
    def __init__(self, outfile, params_array):
        self.__code = ""
        self.__step = " "*4
        self.__outfile = outfile
        self.__params_array = params_array
    def __del__(self):
        pass
    def __addcode(self, step, ecode):
#        if len(ecode) > 80:
        self.__code += self.__step * step + ecode + "\n"
    def __printcode(self):
        if len(self.__outfile) > 0:
#            if os.path.exists(self.__outfile) :
#                print "outfile exists, check the ",self.__outfile
            f = open(self.__outfile, "w")
            print >> f, self.__code
        else:
            print self.__code
    def __python_type_check(self, paramname, mytype):
        if mytype == "uint32_t" or mytype == "int32_t" or mytype == "uint64_t" or mytype == "int64_t" :
            return "type(" + paramname + ") == int"
        if mytype == "string" :
            return "type(" + paramname + ") == str"
        if mytype == "list":
            return "type(" + paramname + ") == list"
        return None
    def parseapi(self, classlist):
        self.__addcode(0, "# this is auto generated code by api-parser")
        self.__addcode(0, "# you are not supposed to edit this file");
        self.__addcode(0, "# feel free to mail scenbuffalo@gmail.com\n");
        self.__addcode(0, "# import xhead_json\n");
        for c in classlist:
            iclassname = c['name']
            iclassdesc = c['methodlist']
            self.__addcode(0, "class "+iclassname+":");
            self.__addcode(1, "def __init__(self):");
            self.__addcode(2, "pass");
            for m in iclassdesc:
                imethodname = m['name']
                imethoddesc = m['paramlist']
                parampurelist = ""
                paramlist = ""
                balanceparam = ""
                line = ""
                for p in imethoddesc:
                    iparamname = p['name']
                    iparamdesc = p
                    if iparamdesc.has_key('default'):
                        pass
                    else:
                        paramlist += iparamname + ", "
                        parampurelist += iparamname + ", "
                    if iparamdesc.has_key('balance') and iparamdesc['balance']:
                        balanceparam = iparamname

                for p in imethoddesc:
                    iparamname = p['name']
                    iparamdesc = p
                    if iparamdesc.has_key('default') and not iparamdesc.has_key('hide'):
                        paramlist += iparamname + " = "+iparamdesc['default']+", "
                        parampurelist += iparamname + ", "
                    else:
                        pass
                # php is so tough
                parampurelist = parampurelist.rstrip()[0:-1]
                paramlist = paramlist.rstrip()[0:-1]

                real_param = self.__params_array and "arrParam" or parampurelist
                self.__addcode(1, "def "+"__check"+imethodname+"Param(self, "+real_param+"):");
                line = "return 1"
                for p in imethoddesc:
                    iparamname = self.__params_array and "arrParam['"+p['name']+"']" or p['name']
                    iparamdesc = p
                    typecheck = self.__python_type_check(iparamname, iparamdesc['type'])
                    if typecheck and not iparamdesc.has_key('hide'):
                        line += " and " + typecheck
                line += ";"
                self.__addcode(2, line)

                real_param = self.__params_array and "arrParam" or paramlist
                self.__addcode(1, "def "+imethodname+"Param(self, "+real_param+"):");
                self.__addcode(2, "qPrama = {};")
                self.__addcode(2, "if False == self."+"__check"+imethodname+"Param(" + parampurelist + ") :")
                self.__addcode(3, "return False;")

                for p in imethoddesc:
                    iparamname = self.__params_array and "arrParam['"+p['name']+"']" or p['name']
                    iparamdesc = p
                    if iparamdesc.has_key('default') and iparamdesc.has_key('hide'):
                        self.__addcode(2, "qPrama['"+p['name']+"'] " + "= " + iparamdesc['default'] +";")
                    else:
                        self.__addcode(2, "qPrama['"+p['name']+"'] " + "= " + iparamname +";")
                self.__addcode(2, "return qPrama;")

                real_param = self.__params_array and "arrParam" or paramlist
                self.__addcode(1, "def "+imethodname+"(self, "+real_param+") :")
                real_param = self.__params_array and "arrParam" or parampurelist
                self.__addcode(2, "qPrama = self."+imethodname+"Param("+real_param+");")
                self.__addcode(2, "if False == qPrama:")
                self.__addcode(3, "return False;")
                self.__addcode(2, "#xheadj = new xhead_json('"+iclassname+"');")
                self.__addcode(2, "#return xheadj->talk(qPrama);")

        self.__printcode()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print "python idl2php.py idlfile [paramsarray: 0|1]"
        sys.exit(1)
    myparams_array = 0
    if len(sys.argv) == 3:
        myparams_array = int(sys.argv[2])
    ft = sys.argv[1].split('.')
    if len(ft) < 2 or ft[-1] != "api":
        print "idlfile nameformat error"
        sys.exit(1)
    out = ""
    for t in ft[0:-1]:
        out += str(t).capitalize() + '.'
    out += "py"
    cgen = CodeGen(outfile = out, params_array = myparams_array )
    cgen.parseapi(api_parse.parse(sys.argv[1]))
