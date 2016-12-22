compiler_exe:	Parser_Code/parser_code.y  Parser_Code/rules.l compiler_code.o symbol_table.o
	bison -d -Dparse.trace Parser_Code/parser_code.y
	flex -oParser_Code/lex.yy.c Parser_Code/rules.l
	cc -o $@ parser_code.tab.c Parser_Code/lex.yy.c Parser_Code/compiler_code.o Parser_Code/symbol_table.o -lfl
	rm Parser_Code/lex.yy.c Parser_Code/compiler_code.o Parser_Code/symbol_table.o parser_code.tab.c parser_code.tab.h

symbol_table.o:	Parser_Code/symbol_table.c Includes/symbol_table.h
	cc -c Parser_Code/symbol_table.c -o Parser_Code/symbol_table.o


compiler_code.o:	Parser_Code/compiler_code.c symbol_table.o
	cc -c Parser_Code/compiler_code.c -o Parser_Code/compiler_code.o
