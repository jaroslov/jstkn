MAKEFLAGS	+=	--no-builtin-rules
OPTLVL		?= -g

.PHONY		: all jstkn

all			: bin/jstkn

bin/jstkn	: lib/jstkn.h src/json.cpp
	@if [ ! -d ./bin ]; then mkdir -p ./bin; fi;
	clang++ $(OPTLVL) -std=c++11 -Ilib src/json.cpp -o ./bin/jstkn

clean		:
	rm -rf bin
