
compiler.exe:	Parser_Code/parser_code.y  Parser_Code/rules.l Parser_Code/compiler_code.o Parser_Code/symbol_table.o 
	bison -d -Dparse.trace Parser_Code/parser_code.y
	flex -oParser_Code/lex.yy.c Parser_Code/rules.l
	cc -o $@  parser_code.tab.c Parser_Code/lex.yy.c Parser_Code/compiler_code.o Parser_Code/symbol_table.o -lfl
	rm Parser_Code/lex.yy.c Parser_Code/compiler_code.o Parser_Code/symbol_table.o parser_code.tab.c parser_code.tab.h 



symbol_table.o:	Parser_Code/symbol_table.c Includes/symbol_table.h Includes/data_types.h Includes/data_types.c 
	cc -c Parser_Code/symbol_table.c -o Parser_Code/symbol_table.o


compiler_code.o:	Parser_Code/compiler_code.c Parser_Code/symbol_table.o Includes/compiler_header.h
	cc -c Parser_Code/compiler_code.c -o Parser_Code/compiler_code.o
