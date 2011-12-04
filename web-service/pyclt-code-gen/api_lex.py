# ----------------------------------------------------------------------
# idl_lex.py
#
# A lexer for IDL.
# ----------------------------------------------------------------------

import sys
sys.path.insert(0,"../..")

import ply.lex as lex

# Reserved words
reserved = (
    'CLASS', 'METHOD',
    'UINT32_T', 'INT32_T', 'UINT64_T', 'INT64_T', 'STRING', 'BINARY', 'DOUBLE',
    'BALANCE', 'DEFAULT',
    )

tokens = reserved + (
    # Literals (identifier, integer constant, float constant, string constant, char const)
    'ID', 'ICONST', 'FCONST', 'SCONST', 'CCONST',

    # Delimeters ( ) [ ] { } ; = == ,
    'LPAREN', 'RPAREN',
    'LBRACKET', 'RBRACKET',
    'LBRACE', 'RBRACE',
    'SEMI', 'EQUALS', 'EEQUALS', 'COMMA',
    )

# Completely ignored characters
t_ignore           = ' \t\x0c'

# Newlines
def t_NEWLINE(t):
    r'\n+'
    t.lexer.lineno += t.value.count("\n")

# Delimeters
t_LPAREN           = r'\('
t_RPAREN           = r'\)'
t_LBRACKET         = r'\['
t_RBRACKET         = r'\]'
t_LBRACE           = r'\{'
t_RBRACE           = r'\}'
t_SEMI             = r';'
t_COMMA            = r','
# Assignment operators
t_EQUALS           = r'='
t_EEQUALS          = r'=='

# Identifiers and reserved words

reserved_map = { }
for r in reserved:
    reserved_map[r.lower()] = r

def t_ID(t):
    r'[A-Za-z_][\w_]*'
    t.type = reserved_map.get(t.value,"ID")
    return t

# Integer literal
t_ICONST = r'\d+([uU]|[lL]|[uU][lL]|[lL][uU])?'

# Floating literal
t_FCONST = r'((\d+)(\.\d+)(e(\+|-)?(\d+))? | (\d+)e(\+|-)?(\d+))([lL]|[fF])?'

# String literal
t_SCONST = r'\"([^\\\n]|(\\.))*?\"'

# Character constant 'c' or L'c'
t_CCONST = r'(L)?\'([^\\\n]|(\\.))*?\''

# Comments
def t_comment(t):
	r'/\*(.|\n)*?\*/|//.*|\#.*'
	t.lexer.lineno += t.value.count('\n')

def t_error(t):
	print("Illegal character %s" % repr(t.value[0]))
	t.lexer.skip(1)

def t_preprocessor(t):
	r'\#(.)*?\n'
	t.lexer.lineno += 1

lexer = lex.lex(optimize=1)
if __name__ == "__main__":
    lex.runmain(lexer)
