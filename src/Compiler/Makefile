compiler_test:	parser_code.y  rules.l
				bison -d parser_code.y
				flex rules.l
				cc -o $@ parser_code.tab.c lex.yy.c -lfl
