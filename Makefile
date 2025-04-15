all: build

svr:
	$(MAKE) -C server

clnt:
	$(MAKE) -C client

build: svr clnt

clean:
	$(MAKE) -C client clean
	$(MAKE) -C server clean
