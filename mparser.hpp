#ifndef _MPARSER_H_
#define _MPARSER_H_

#include "strmap.hpp"
#include "lexer.hpp"

#define MAX_STACK_SIZE 64

extern const double DblErR;
extern const double DblNiN;


typedef double (*OneArgFunc) ( double arg );
typedef char* (*MultiArgFunc) ( int paramcnt, double *args,
                                m_str_map *strparams, double *result );
typedef int (*PrmSrchFunc) ( const char *str, int len, double *value,
                             void *param );

class mparser {
public:
    mparser( char *MoreLetters=nullptr );
    char* parse(const char *formula, double *result);
    ~mparser();
    m_str_map parameters;
    m_str_map ExtFunctions;
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
    } oper_type_t;
    static const char op_priorities[OP_FUNC_MULTIARG+1];
    typedef struct {
        oper_type_t OperType;
        void      *Func;
        char       PrevValTop;
        m_str_map   *StrParams;
    } Operation;
    static const Operation br_op;
    static const Operation neg_op;
    Operation OpStack[MAX_STACK_SIZE];
    double  ValStack[MAX_STACK_SIZE];
    int OpTop, ValTop;
    int ObrDist;
    static int ref_counter;
    mlexer lexer;
    int script;
    m_str_map *VarParams;
    static m_str_map int_functions;
    static m_math_sym_table math_sym_table;
    static char errbuf[256];
    static hq_char_type_t math_char_type_table[256];
    static int initializations_performed;
    char* parse_script(double *result);
    char* parse_formula(double *result);
    char* prepare_formula();
    char* calc();
    char* calc_to_obr();
};

#endif //_MPARSER_H_
