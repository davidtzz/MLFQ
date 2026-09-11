all: scheduler

clean:
	rm -f scheduler test_runner

scheduler:
	gcc -o scheduler main.c process.c scheduler.c queue.c

test:
	bash tests/run_tests.sh
