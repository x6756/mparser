#include "symtable.hpp"
#include <cstring>
#include <cstdlib>

static char math_symbols[] =
    "\033\002" "<<" ">>" "**" "<>" ">=" "<=" "&&" "||" "/*" ":="
    "\033\001" "(+-*/%$^~&|=><?:),;";

m_sym_table::~m_sym_table() {
    for(int i = 0; i < 256; ++i) {
        free(table[i]);
    }
}

void m_sym_table::prepare_symbols(char *symbols) {
    int i = 0, nchars = 1;
    memset(table, 0, 256 * sizeof(symbol_rec_t*));
    while (*symbols) {
        if (*symbols=='\033') {
            nchars = *++symbols;
            ++symbols;
        } else {
            symbol_rec_t **RecList = &table [*symbols];
            symbol_rec_t *Rec = *RecList;
            int count = 0;
            while( Rec ) {
                ++count;
                if ( Rec->more ) {
                    ++Rec;
                }
                else {
                    break;
                }
            }
            if (Rec) {
                *RecList = (symbol_rec_t*)realloc( *RecList, (count+1)*sizeof(symbol_rec_t));
                if(*RecList == nullptr) {
                    throw("realloc in prepare_symbols failed");
                }
                Rec = *RecList + count;
                (Rec-1)->more = 1;
            } else {
                symbol_rec_t* R = (symbol_rec_t*) malloc( 7 );
                *RecList=R;
                Rec = *RecList;
            }
            strncpy( Rec->sym, symbols, 4 );
            Rec->len = (char) nchars;
            Rec->index = (char) i;
            Rec->more = 0;
            symbols += nchars;
            ++i;
        }
    }
}

int m_sym_table::find_symbol(char *str, int *nchars) {
    symbol_rec_t *rec = table[ (int)*str ];
    while(rec) {
        if ((rec->len == 1 && rec->sym[0] == str[0])                            ||
            (rec->len == 2 && rec->sym[0] == str[0] && rec->sym[1] == str[1])   ||
            (rec->len == 3 && rec->sym[0] == str[0] && rec->sym[1] == str[1]    &&
             rec->sym[2] == str[2])) {
            *nchars = rec->len;
            return rec->index;
        }
        rec = (rec->more) ? rec + 1 : nullptr;
    }
    return -1;
}

m_math_sym_table::m_math_sym_table() {
    prepare_symbols(math_symbols);
}
