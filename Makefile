# NAME: Ruth Johnson
# EMAIL: ruthjohnson@ucla.edu
# ID: 704275412

CC=gcc

default:
	$(CC)  -lmraa -lm -o lab4c_tcp lab4c_tcp.c
	$(CC)  -lmraa -lssl -lcrypto -lm -o lab4c_tls lab4c_tls.c

tcp:
	$(CC)  -lm -o lab4c_tcp lab4c_tcp.c

tls: 
	$(CC)  -lssl -lcrypto -lm -o lab4b_tls lab4b_tls.c
check:
	bash test_script.sh
clean:
	rm lab4c-704275412.tar.gz
	rm lab4c_tcp
	rm lab4c_tls


dist:
	tar -czvf lab4c-704275412.tar.gz lab4c_tls.c lab4c_tcp.c  README Makefile
