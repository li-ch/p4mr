preproc:	  preprocessor_parser.y preprocessor_rules.l
	bison -d  preprocessor_parser.y
	flex -o preproc_rules.y.c preprocessor_rules.l
	gcc -o $@ preprocessor_parser.tab.c preproc_rules.y.c -lfl

clean:	
	rm preproc preprocessor_parser.tab.h preprocessor_parser.tab.c preproc_rules.y.c
