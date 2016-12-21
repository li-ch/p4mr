compiler_test:	parser_code.y  rules.l compiler_code.o symbol_table.o
	bison -d -Dparse.trace  parser_code.y
	flex rules.l
	cc -o $@ parser_code.tab.c lex.yy.c compiler_code.o symbol_table.o -lfl


symbol_table.o:	symbol_table.c symbol_table.h
	cc -c symbol_table.c


compiler_code.o:	compiler_code.c symbol_table.o
	cc -c compiler_code.c 

clean:
	rm parser_code.tab.c parser_code.tab.h lex.yy.c compiler_test compiler_code.o symbol_table.o
