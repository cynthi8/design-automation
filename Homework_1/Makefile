all: klprog saprog

klprog:
	$(MAKE) -C ./KL
	cp ./KL/klprog ./

saprog:
	$(MAKE) -C ./SA
	cp ./SA/saprog ./

clean:
	$(MAKE) -C ./KL clean
	$(MAKE) -C ./SA clean
	rm -f klprog saprog