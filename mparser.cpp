#define  _USE_MATH_DEFINES
#include <cmath>
#include <climits>
#include <cstdio>
#include <cstring>
#include "mparser.hpp"

static const double DblErR = -1.68736462823243E308;
static const double DblNiN = -1.68376462823243E308;

static char eBrackets [] = "Brackets don't match";
static char eSyntax   [] = "Syntax error";
static char eInternal [] = "Internal error";
static char eExtraOp  [] = "Extra operation";
static char eInfinity [] = "Infinity somewhere";
static char eUnknFunc [] = "%s - Unknown function/variable";
static char eLogicErr [] = "Logical expression error";
static char eUnexpEnd [] = "Unexpected end of script";
static char eExpVarRet[] = "Variable name or return expected";
static char eExpAssign[] = "Assignment expected";
static char eValSizErr[] = "Value too big for operation";
static char eInvPrmCnt[] = "Invalid parameters count for function call";

static char std_symbols[] = "+-/*^~()<>%$,?:=&|;";

static char func_names[] =
    "ATAN\000COS\000SIN\000TAN\000ABS\000"
    "EXP\000LN\000LG\000SQRT\000FRAC\000"
    "TRUNC\000FLOOR\000CEIL\000ROUND\000ASIN\000"
    "ACOS\000SGN\000NEG\000E\000PI\000";

//Indexes of some functions in func_names[] array
#define FUNC_ROUND  13
#define FUNC_E      18
#define FUNC_PI     19

static double _neg_(const double);
static double _frac_(const double);
static double _trunc_(double);
static double _sgn_(double);
static char* _round_( int paramcnt, double *args, m_str_map *strparams, double *result );

typedef double (*dfd)(double);
static dfd func_addresses[]= {
    &atan, &cos, &sin, &tan, &fabs,
    &exp, &log, &log10, &sqrt, &_frac_,
    &_trunc_, &floor, ceil, (double(*)(double)) &_round_, &asin,
    &acos, &_sgn_, &_neg_, nullptr, nullptr
};

inline void type_table_add_chars( hq_char_type_t *CharTypeTable, char *Symbols,
                               hq_char_type_t CharType ) {
    while (*Symbols) {
        CharTypeTable[ (uchar) *Symbols++] = CharType;
    }
}

const char mparser::op_priorities[mparser::OP_FUNC_MULTIARG+1] = {
    5, 5, 5,    2, 2, 2, 2, 2,    -1, -1,   0,
    3, 3,    4, 4, 4, 4,
    5, 5, 5, 5,    2, 2, 2,   1, 2, 0, 2,
    -1, 6, 6
};

m_str_map mparser::int_functions;

char mparser::errbuf[256];

const mparser::Operation mparser::br_op = {OP_OBR};

const mparser::Operation mparser::neg_op = { OP_FUNC_ONEARG, (void*)&_neg_, 0, nullptr };

int mparser::initializations_performed = 0;

hq_char_type_t mparser::math_char_type_table[256];

int mparser::ref_counter = 0;

m_math_sym_table mparser::math_sym_table;


mparser::mparser( char *MoreLetters):ExtFunctions(sizeof(void*)) {
    if (ref_counter++ == 0) {
        // init character tables
        init_char_type_table( math_char_type_table,CH_LETTER | CH_DIGIT | CH_SEPARAT | CH_QUOTE );
        type_table_add_chars( math_char_type_table, std_symbols, CH_SYMBOL );
        if (MoreLetters) {
            type_table_add_chars( math_char_type_table, MoreLetters, CH_LETTER );
        }
        // init function maps
        int_functions.create_from_chain( sizeof(void*), (char*)func_names, func_addresses );
        initializations_performed = 1;
    }
    ParamFuncParam = nullptr;
    MoreParams = nullptr;
    VarParams = nullptr;
    lexer.NoIntegers = 1;
    lexer.SymTable = &math_sym_table;
    lexer.CharTypeTable = math_char_type_table;
    lexer.cssn = 8;
    lexer.ComEnd = (char*) "*/";
}

mparser::~mparser() {}

char* mparser::prepare_formula() {
    int BrCnt = 0;
    char *SS = lexer.get_current_pos();

    // Brackets Matching
    while ( (!script && *SS) || (script && *SS != ';') ) {
        if (*SS=='(') {
            ++BrCnt;
        }
        else if (*SS==')' && --BrCnt<0) {
            goto brkerr;
        }
        ++SS;
    }
    if (BrCnt != 0) {
brkerr:
        return eBrackets;
    }

    OpTop = 0;
    ValTop = -1;
    OpStack[0].OperType = OP_OBR;
    ObrDist = 2;
    return nullptr;
}

char* mparser::parse(const char *formula, double *result) {
    if (!formula || !*formula) {
        *result = 0.0;
        return nullptr;
    }

    script = *formula == '#' && *(formula+1) == '!' && math_char_type_table[ (uchar)*(formula+2) ] == CH_SEPARAT;

    if (script) {
        formula += 3;
    }

    lexer.set_parse_string(formula);

    return script ? parse_script(result) : parse_formula(result);
}

char* mparser::parse_formula( double *result ) {
    char *ErrorMsg;

    if ((ErrorMsg = prepare_formula()) != nullptr ) {
        return ErrorMsg;
    }

    hq_token_type_t ToTi = lexer.get_next_token();
    for (;;) {
        --ObrDist;
        switch (ToTi) {
        case TOK_ERROR:
            return eSyntax;
        case TOK_FINAL:
formula_end:
            if ((ErrorMsg = calc_to_obr()) != nullptr) {
                return ErrorMsg;
            }
            goto getout;
        case TOK_FLOAT:
            ValStack[++ValTop] = lexer.ExtValue;
            break;
        case TOK_SYMBOL:
            switch ( lexer.IntValue ) {
            case OP_OBR:    // (
                OpStack[++OpTop] = br_op;
                ObrDist = 2;
                break;
            case OP_CBR:    // )
                if ((ErrorMsg = calc_to_obr()) != nullptr) {
                    return ErrorMsg;
                }
                break;
            case OP_COMMA: {    // ,
                if ( (ErrorMsg = calc_to_obr()) != nullptr ) {
                    return ErrorMsg;
                }

                Operation *pOp;

                if ((pOp = &OpStack[OpTop])->OperType == OP_FUNC_MULTIARG) {
                    OpStack[++OpTop] = br_op;
                    ObrDist = 2;
                } else {
                    return eSyntax;
                }
                break;
            }
            default: {
                Operation Op;
                Op.OperType = (oper_type_t) lexer.IntValue;
                switch (Op.OperType) {
                    case OP_FORMULAEND:
                        if (script) {
                            goto formula_end;
                        }
                        else {
                            return eSyntax;
                        }
                    case OP_ADD:
                        if (ObrDist >= 1) {
                            goto next_tok;
                        }
                    break;
                    case OP_SUB:
                        if (ObrDist >= 1) {
                            OpStack[++OpTop] = neg_op;
                            goto next_tok;
                        }
                    break;
                    case OP_LOGIC:
                    case OP_LOGIC_SEP:
                        ObrDist = 2;
                    break;
                    default:
                    break;
                }
                while(op_priorities[ Op.OperType ] <= op_priorities[ OpStack[OpTop].OperType ]) {
                    if((ErrorMsg = calc()) != nullptr) {
                        return ErrorMsg;
                    }
                }
                OpStack[++OpTop] = Op;
                break;
            }
            }
            break;
        case TOK_NAME: {
            Operation Op;
            double *value, dblval;
            void **func;
            long long funcnum, namelen = lexer.NameLen;

            char *SS = lexer.Name;
            for(int i = namelen; i>0; --i) {
                *SS++ = toupper((int)*SS);
            }

            funcnum = int_functions.index_of(lexer.Name,lexer.NameLen,(void**) &func);
            if (funcnum >= 0) {
                Op.Func = *func;
                switch (funcnum) {
                case FUNC_E:
                    ValStack[++ValTop] = M_E;
                    break;
                case FUNC_PI:
                    ValStack[++ValTop] = M_PI;
                    break;
                case FUNC_ROUND:
                    Op.OperType = OP_FUNC_MULTIARG;
                    Op.PrevValTop = ValTop;
                    Op.StrParams = nullptr;
                    OpStack[++OpTop] = Op;
                    break;
                default:// Internal function
                    Op.OperType = OP_FUNC_ONEARG;
                    OpStack[++OpTop] = Op;
                }
            } else if (parameters.index_of(lexer.Name,lexer.NameLen,(void**) &value )>= 0) {
                if (*value==DblErR) {
                    return eInternal;
                } else {
                    ValStack[++ValTop] = *value;
                }
            } else if (ExtFunctions.index_of(lexer.Name,lexer.NameLen,(void**) &func ) >= 0) {
                Op.Func = *func;
                Op.OperType = OP_FUNC_MULTIARG;
                Op.PrevValTop = ValTop;
                Op.StrParams = nullptr;
                OpStack[++OpTop] = Op;
            } else if (VarParams
                       &&
                       VarParams->index_of(lexer.Name,lexer.NameLen,(void**) &value )    >= 0
                      ) {
                if (*value==DblErR) {
                    return eInternal;
                } else {
                    ValStack[++ValTop] = *value;
                }
            } else if (MoreParams
                       &&
                       (*MoreParams)( lexer.Name,
                                      lexer.NameLen,
                                      &dblval,
                                      ParamFuncParam )
                      ) {
                ValStack[++ValTop] = dblval;
            } else {
                char buf[256];
                strncpy( buf, lexer.Name, lexer.NameLen );
                buf[lexer.NameLen] = '\0';
                sprintf( errbuf, eUnknFunc, buf );
                return errbuf;
            }
            break;
        }
        case TOK_STRING: {
            if (OpTop < 1) {
                return eSyntax;
            }

            Operation *pOp;

            if ( (pOp = &OpStack[OpTop-1])->OperType
                    == OP_FUNC_MULTIARG ) {
                if (!pOp->StrParams) {
                    pOp->StrParams = new m_str_map(0);
                }
                pOp->StrParams->add_str(lexer.Name,lexer.NameLen, nullptr );
                ValStack[++ValTop] = DblNiN;
            } else {
                return eSyntax;
            }
            break;
        }
        default:
            return eSyntax;
        }
next_tok:
        ToTi = lexer.get_next_token();
    } // forever

getout:
    if (OpTop != -1 || ValTop != 0) {
        return eInternal;
    }

    *result = ValStack[0];
    return nullptr;
}

char* mparser::parse_script(double *result) {
    char *ErrorMsg = nullptr;
    int expectvar = 1, was_return = 0;
    char *varname = nullptr;
    long long varnamelen = 0;

    VarParams = new m_str_map();

    hq_token_type_t ToTi = lexer.get_next_token();
    for (;;) {
        switch (ToTi) {
        case TOK_FINAL:
            ErrorMsg = eUnexpEnd;
            goto getout;
        case TOK_NAME: {
            if (!expectvar) {
                ErrorMsg = eExpVarRet;
                goto getout;
            } else {
                char *SS = lexer.Name;

                varnamelen = lexer.NameLen;

                for(int i = varnamelen; i>0; --i){
                    *SS++ = toupper((int)(uchar)*SS);
                }
            }
            varname = lexer.Name;

            was_return = strncmp(varname, "RETURN", varnamelen) == 0;
            if (was_return) {
                ErrorMsg = parse_formula(result);
                goto getout;
            }
            expectvar = 0;
            break;
        }
        case TOK_SYMBOL: {
            if (lexer.IntValue != OP_ASSIGN || expectvar) {
                ErrorMsg = eExpAssign;
                goto getout;
            }
            ErrorMsg = parse_formula(result);
            if (ErrorMsg) {
                goto getout;
            }

            double *value;
            if (VarParams->index_of(varname, varnamelen,(void**) &value ) >= 0 ) {
                *value = *result;
            }
            else {
                VarParams->add_str(varname, varnamelen, result);
            }
            expectvar = 1;
            break;
        }
        default:
            ErrorMsg = eSyntax;
            goto getout;
        }
        ToTi = lexer.get_next_token();
    }

getout:
    delete VarParams;
    return ErrorMsg;
}

char* mparser::calc()
{
    double Res;
    Operation Op = OpStack[OpTop--];

    // multi-argument external or internal fucntion
    if ( Op.OperType == OP_FUNC_MULTIARG ) {
        int paramcnt = ValTop - Op.PrevValTop;
        char *ErrorMsg;
#ifdef MDEBUG
        printf( "ValTop = %d, OpTop = %d, PrevValTop = %d\n",
                ValTop, OpTop, Op.PrevValTop );
#endif
        ValTop = Op.PrevValTop;
        ErrorMsg = (*(MultiArgFunc)Op.Func)( paramcnt,
                                             &ValStack[ValTop+1],
                                             Op.StrParams, &Res );
        if (ErrorMsg) {
            return ErrorMsg;
        }
        if (Op.StrParams) {
            delete Op.StrParams;
        }
        ValStack[++ValTop] = Res;
#ifdef MDEBUG
        printf("ValTop = %d, OpTop = %d\n", ValTop, OpTop );
#endif
        return nullptr;
    }

    if (Op.OperType==OP_LOGIC) {
        return nullptr;
    }

    // get right arg
    if (ValTop<0) {
        return eExtraOp;
    }
    double ValR = ValStack[ValTop--];

    // one arg operations
    if (Op.OperType==OP_NOT) {
        if (ValR >= INT_MIN && ValR <= INT_MAX) {
            Res = ~((int) ValR);
        }
        else {
            return eValSizErr;
        }
    } else if (Op.OperType==OP_FUNC_ONEARG) {
        Res = (*(OneArgFunc)Op.Func)( ValR );
    } else {
        // get left arg
        if (ValTop<0) {
            return eExtraOp;
        }
        double ValL = ValStack[ValTop--];
        switch (Op.OperType) {
        // Binary
        case OP_SHL:
            if (ValL >= INT_MIN && ValL <= INT_MAX && ValR >= INT_MIN && ValR <= INT_MAX) {
                Res = (int) ValL << (int) ValR;
            }
            else {
                return eValSizErr;
            }
            break;
        case OP_SHR:
            if (ValL >= INT_MIN && ValL <= INT_MAX && ValR >= INT_MIN && ValR <= INT_MAX) {
                Res = (int) ValL >> (int) ValR;
            }
            else {
                return eValSizErr;
            }
            break;
        case OP_POW:
            Res = pow( ValL, ValR );
            break;
        // Logical
        case OP_LOGIC_NEQ:
            Res = ValL != ValR;
            break;
        case OP_LOGIC_GEQ:
            Res = ValL >= ValR;
            break;
        case OP_LOGIC_LEQ:
            Res = ValL <= ValR;
            break;
        case OP_LOGIC_AND:
            Res = ValL && ValR;
            break;
        case OP_LOGIC_OR:
            Res = ValL || ValR;
            break;
        // Arithmetic
        case OP_ADD:
            Res = ValL + ValR;
            break;
        case OP_SUB:
            Res = ValL - ValR;
            break;
        case OP_MUL:
            Res = ValL * ValR;
            break;
        case OP_DIV:
            if (ValR == 0.0) {
                return eInfinity;
            }
            Res = ValL / ValR;
            break;
        case OP_MOD:
            Res = fmod(ValL, ValR);
            break;
        case OP_UNK:
            if (ValL<=0) {
                Res = 0.0;
            }
            else if (ValR==0.0) {
                return eInfinity;
            }
            else {
                Res = ceil(ValL / ValR);
            }
            break;
        // Bitwise
        case OP_XOR:
            if (ValL >= INT_MIN && ValL <= INT_MAX && ValR >= INT_MIN && ValR <= INT_MAX) {
                Res = (int) ValL ^ (int) ValR;
            }
            else {
                return eValSizErr;
            }
            break;
        case OP_AND:
            if (ValL >= INT_MIN && ValL <= INT_MAX && ValR >= INT_MIN && ValR <= INT_MAX) {
                Res = (int) ValL & (int) ValR;
            }
            else {
                return eValSizErr;
            }
            break;
        case OP_OR:
            if (ValL >= INT_MIN && ValL <= INT_MAX && ValR >= INT_MIN && ValR <= INT_MAX) {
                Res = (int) ValL | (int) ValR;
            }
            else {
                return eValSizErr;
            }
            break;
        // Logical
        case OP_EQU:
            Res = ValL == ValR;
            break;
        case OP_GREATER:
            Res = ValL > ValR;
            break;
        case OP_LESS:
            Res = ValL < ValR;
            break;
        case OP_LOGIC_SEP: {
            // needs three arguments
            if (OpTop < 0 || OpStack[OpTop--].OperType != OP_LOGIC) {
                return eLogicErr;
            }
            double ValLL = ValStack[ ValTop-- ];
            Res = ValLL ? ValL : ValR;
            break;
        }
        default:
            return eInternal;
        }
    }
    ValStack[++ValTop] = Res;
    return nullptr;
}

char* mparser::calc_to_obr() {
    while ( OpStack[OpTop].OperType != OP_OBR ) {
        char *ErrorMsg;
        if ( (ErrorMsg = calc()) != nullptr ) {
            return ErrorMsg;
        }
    }
    --OpTop;
    return nullptr;
}

static double _frac_(const double x) {
    double y;
    return modf(x, &y);
}

static double _trunc_(const double x){
    return (x >= 0.0) ? floor(x) : ceil(x);
}

static double _sgn_(const double x){
    return (x > 0) ? 1 : (x < 0) ? -1 : 0;
}

static double _neg_(const double x){
    return -x;
}

/* "Advanced" round function; second argument - sharpness */
static char* _round_(int paramcnt, double *args, [[maybe_unused]] m_str_map *strparams, double *result ) {
    int sharpness;
    double x, coef;

    if (paramcnt == 1) {
        sharpness = 0;
    }
    else if (paramcnt == 2) {
        sharpness = (int) args[1];
    }
    else {
        return eInvPrmCnt;
    }

    x = args[0];
    if (sharpness < 0) {
        coef = 0.1;
        sharpness = -sharpness;
    } else {
        coef = 10;
    }

    for (int i = 0; i < sharpness; ++i) {
        x *= coef;
    }

    x = (x + ( (x >= 0) ? 0.5 : -0.5 ) );
    if (x >= 0.0) {
        x = floor(x);
    }
    else {
        x = ceil(x);
    }

    for (int i = 0; i < sharpness; ++i) {
        x /= coef;
    }

    *result = x;

    return nullptr;
}
