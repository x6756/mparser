#ifndef _SYMTABLE_H_
#define _SYMTABLE_H_

class m_sym_table {
public:
    struct symbol_rec_t {
        char sym[4];
        char len;
        char index;
        char more;
    };
    virtual ~m_sym_table();

    void prepare_symbols( char *symbols );
    int find_symbol( char *str, int *nchars );
protected:
    m_sym_table() {};
private:
    symbol_rec_t* table[256];
};

class m_math_sym_table: public m_sym_table {
public:
    m_math_sym_table();
    virtual ~m_math_sym_table() {};
};


#endif
