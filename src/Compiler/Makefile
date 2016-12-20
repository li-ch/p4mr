compiler_test:	parser_code.y  rules.l compiler_code.o
	bison -d -Dparse.trace  parser_code.y
	flex rules.l
	cc -c compiler_code.c
	cc -o $@ parser_code.tab.c lex.yy.c compiler_code.o -lfl

clean:
	rm parser_code.tab.c parser_code.tab.h lex.yy.c compiler_test compiler_code.o
