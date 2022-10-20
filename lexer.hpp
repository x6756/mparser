#ifndef _LEXER_H_
#define _LEXER_H_
#include <cstdlib>

#include "symtable.hpp"

typedef unsigned char uchar;

typedef enum {
    CH_LETTER = 0x01, CH_DIGIT = 0x02, CH_SEPARAT = 0x04,
    CH_SYMBOL = 0x08, CH_QUOTE = 0x10,
    CH_UNKNOWN= 0x7E, CH_FINAL = 0x7F
} hq_char_type_t;

typedef enum {
    TOK_ERROR, TOK_NONE, TOK_FINAL, TOK_INT, TOK_FLOAT, TOK_SYMBOL,
    TOK_NAME, TOK_STRING
} hq_token_type_t;

class mlexer {
public:
    // input params
    int     cssn;   // Comment Start Symbol Number. -1 if none
    char    *ComEnd;    // End of comment
    m_sym_table *SymTable;
    hq_char_type_t *CharTypeTable;

    // output params
    char       *Name;
    long long  NameLen;
    double  ExtValue;
    int     IntValue;
    hq_token_type_t PrevTokenType;
    hq_char_type_t  CharType;
    int     NoIntegers;
    int set_parse_string(const char *str );
    hq_token_type_t get_next_token();
    ~mlexer() {
        if(str_) {
            free(str_);
        }
    };
    char* get_current_pos();
    mlexer():str_(nullptr) {
    };
private:
    char *SS, *str_;
};

/* Misc */

void init_char_type_table( hq_char_type_t *CharTypeTable, int CharTypes );

#endif //_LEXER_H_
