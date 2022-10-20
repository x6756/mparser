#include <cstring>
#include <cstring>
#include <climits>
#include "lexer.hpp"

#ifdef MDEBUG
#include <cstdio>
#endif

void init_char_type_table( hq_char_type_t *CharTypeTable, int CharTypes )
{
#ifdef MDEBUG
    printf( "CharTypeTable = 0x%X; CharTypes = %d\n", (unsigned)CharTypeTable,
            CharTypes );
#endif
    memset(CharTypeTable, CH_UNKNOWN, 256 * sizeof(hq_char_type_t));

    CharTypeTable[0] = CH_FINAL;

    if (CharTypes & CH_SEPARAT) {
        CharTypeTable[(int)' '] = CH_SEPARAT;
        CharTypeTable[9]  = CH_SEPARAT;
        CharTypeTable[13]  = CH_SEPARAT;
        CharTypeTable[10]  = CH_SEPARAT;
    }

    if (CharTypes & CH_QUOTE) {
        CharTypeTable[(int)'\'']  = CH_QUOTE;
    }
    int ch;
    if (CharTypes & CH_LETTER) {
        for (ch='A'; ch<='Z'; ++ch) {
            CharTypeTable[ch] = CH_LETTER;
        }
        for (ch='a'; ch<='z'; ++ch) {
            CharTypeTable[ch] = CH_LETTER;
        }
        CharTypeTable[(int)'_'] = CH_LETTER;
    }

    if (CharTypes & CH_DIGIT) {
        for (ch='0'; ch<='9'; ++ch) {
            CharTypeTable[ch] = CH_DIGIT;
        }
    }
}

#define CHARTYPEPP CharTypeTable[ (uchar) *++(SS) ]
#define CHARTYPE CharTypeTable[ (uchar) *SS ]

int mlexer::set_parse_string(const char *str )
{
    PrevTokenType = TOK_NONE;
    if ( !str || !*str ) {
        return 0;
    }

    if(str_) {
        free(str_);
    }
    str_ = (char *)malloc(strlen(str)+1);
    memset(str_,0,strlen(str)+1);
    strncpy(str_, str, strlen(str));
    SS = str_;
    CharType = CHARTYPE;
    return 1;
}

hq_token_type_t mlexer::get_next_token()
{
    hq_token_type_t result = TOK_ERROR;

next_token:

    while ( CharType == CH_SEPARAT ) {
        CharType = CHARTYPEPP;
    }

    switch ( CharType ) {
    case CH_FINAL:
        result = TOK_FINAL;
        break;
    case CH_LETTER:
        Name = SS;
        do {
            CharType = CHARTYPEPP;
        } while (CharType <= CH_DIGIT);
        NameLen = SS - Name;
        result = TOK_NAME;
        break;
    case CH_DIGIT: {
        char *NewSS;
        if ( *SS == '0' && *(SS+1) == 'x' ) {
            IntValue = strtol( SS, &NewSS, 16 );
            if ( SS != NewSS ) {
                SS = NewSS;
                if (NoIntegers) {
                    ExtValue = IntValue;
                    result = TOK_FLOAT;
                } else {
                    result = TOK_INT;
                }
                CharType = CHARTYPE;
            }
            break;
        }
        ExtValue = strtod( SS, &NewSS );
        if ( SS != NewSS ) {
            ;
            SS = NewSS;
            if ( !NoIntegers
                    && ExtValue<=INT_MAX
                    && ExtValue>=INT_MAX
                    && (double)( IntValue = (uchar) ExtValue )
                    == ExtValue ) {
                result = TOK_INT;
            } else {
                result = TOK_FLOAT;
            }
            CharType = CHARTYPE;
        }
        break;
    }
    case CH_SYMBOL: {
        int nchars;
        int i = SymTable->find_symbol( SS, &nchars );
        if (i >= 0) {
            SS += nchars;
            if (i == cssn) {
                char comend = *ComEnd;
                char comendpp = *(ComEnd+1);
                while ( *SS ) {
                    if ( *SS == comend
                            &&
                            ( comendpp == '\0' || *(SS+1) == comendpp )
                       ) {
                        ++SS;
                        if (comendpp != '\0') {
                            ++SS;
                        }
                        CharType = CHARTYPE;
                        goto next_token;
                    }
                    ++SS;
                }
                break;
            }
            CharType = CHARTYPE;
            IntValue = i;
            result = TOK_SYMBOL;
        }
        break;
    }
    case CH_QUOTE:
        Name = ++(SS);
        while ( CharTypeTable[ (uchar)*SS ] != CH_QUOTE && *(SS) != '\0' ) {
            ++SS;
        }
        if ( CHARTYPE == CH_QUOTE ) {
            NameLen = SS - Name;
            ++SS;
            CharType = CHARTYPE;
            result = TOK_STRING;
        }
        break;
    default:
        break;
    }
    return PrevTokenType = result;
}

char* mlexer::get_current_pos()
{
    return SS;
}
