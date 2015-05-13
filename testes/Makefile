all: bib testes

testes:
	gcc -o cria_fim cria_fim.c -I/../include -L/../lib -lmthread -Wall
	gcc -o cria_meio cria_meio.c -I/../include -L/../lib -lmthread -Wall
	gcc -o testa_yield testayield.c -I/../include -L/../lib -lmthread -Wall
	
bib:
	ar crs /../lib/libmthread.a /../lib/*.o
		
clean:
	rm *~ *.o cria_meio
	rm *~ *.o cria_fim
	rm *~ *.o testa_yield
