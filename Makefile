all: scheduler

clean:
	rm -f scheduler test_runner

scheduler:
	gcc -o scheduler main.c process.c scheduler.c queue.c

test:
	gcc -o test_runner test_process.c process.c queue.c scheduler.c
	./test_runner
