#ifndef _MPARSER_H_
#define _MPARSER_H_

#include "strmap.hpp"
#include "lexer.hpp"

#define MAX_STACK_SIZE 64

extern const double DblErR;
extern const double DblNiN;


typedef double (*OneArgFunc) ( double arg );
typedef char* (*MultiArgFunc) ( int paramcnt, double *args,
                                MStrMap *strparams, double *result );
typedef int (*PrmSrchFunc) ( const char *str, int len, double *value,
                             void *param );

class MParser {
public:
    MParser( char *MoreLetters=nullptr );
    char* parse(const char *formula, double *result);
    ~MParser();
    MStrMap parameters;
    MStrMap ExtFunctions;
    PrmSrchFunc MoreParams;
    void  *ParamFuncParam;
private:
    typedef enum {
        // Binary
        OP_SHL, OP_SHR, OP_POW,
        OP_LOGIC_NEQ, OP_LOGIC_GEQ, OP_LOGIC_LEQ,
        OP_LOGIC_AND, OP_LOGIC_OR, // Logical
        OP_COMSTART, OP_ASSIGN, // For internal needs
        OP_OBR, // Special
        OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD, OP_UNK, // Arithmetic
        OP_XOR, OP_NOT, OP_AND, OP_OR, // Bitwise
        OP_EQU, OP_GREATER, OP_LESS,
        OP_LOGIC, OP_LOGIC_SEP, OP_CBR, OP_COMMA, // Logical
        OP_FORMULAEND, // For script
        OP_FUNC_ONEARG, OP_FUNC_MULTIARG // Special
    } OperType_t;
    static const char OpPriorities[OP_FUNC_MULTIARG+1];
    typedef struct {
        OperType_t OperType;
        void      *Func;
        char       PrevValTop;
        MStrMap   *StrParams;
    } Operation;
    static const Operation BrOp;
    static const Operation NegOp;
    Operation OpStack[MAX_STACK_SIZE];
    double  ValStack[MAX_STACK_SIZE];
    int OpTop, ValTop;
    int ObrDist;
    static int refCounter;
    MLexer Lexer;
    int script;
    MStrMap *VarParams;
    static MStrMap IntFunctions;
    static m_math_sym_table MathSymTable;
    static char errbuf[256];
    static hqCharType MathCharTypeTable[256];
    static int initializations_performed;
    char* ParseScript(double *result);
    char* ParseFormula(double *result);
    char* PrepareFormula();
    char* Calc();
    char* CalcToObr();
};

#endif //_MPARSER_H_
