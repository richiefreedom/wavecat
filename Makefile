CFLAGS = -std=gnu99 -O2 -rdynamic -Iinclude -Ithirdparty/jsmn -Werror -Wextra -pedantic -lm
LDFLAGS = -Lthirdparty/jsmn -lm -ljsmn -lpthread -ldl
FILES = main.c \
	  lib/interface/json_input.c \
	  lib/net/url.c \
	  lib/plugin/plugin.c \
	  lib/integration/runge_kutta.c \
	  lib/integration/cmplx_runge_kutta.c \
	  catastrophe/catastrophe_common.c \
	  catastrophe/catastrophe_parallel.c \
	  catastrophe/catastrophe_Asub3.c \
	  catastrophe/catastrophe_Asub1sup4.c \
	  catastrophe/catastrophe_Ksub4_2.c \
	  catastrophe/cmplx_catastrophe_Asub3.c \
	  catastrophe/cmplx_catastrophe_Csub3.c \
	  catastrophe/cmplx_catastrophe_Esub6.c \
	  catastrophe/cmplx_catastrophe_Psub8.c \
	  catastrophe/cmplx_catastrophe_Fsub1_0.c \
	  catastrophe/cmplx_catastrophe_Csub4.c \
	  catastrophe/cmplx_catastrophe_Fsub4.c \
	  catastrophe/cmplx_catastrophe_Asub1sup4.c \
	  catastrophe/cmplx_catastrophe_Bsub3.c \
	  catastrophe/cmplx_catastrophe_Dsub4.c \
	  catastrophe/cmplx_catastrophe_Asub1Asub2Asub1Asub1.c

all: tplibs
	gcc $(CFLAGS) $(FILES) -o wavecat.exe $(LDFLAGS)

tplibs:
	make -C thirdparty/jsmn

thirdpartyclean:
	make clean -C thirdparty/jsmn

server:
	make -C thirdparty/mongoose/examples/web_server

serverclean:
	make clean -C thirdparty/mongoose/examples/web_server

test: all server
	cp wavecat.exe web/
	thirdparty/mongoose/examples/web_server/web_server -document_root ../../../../web -cgi_pattern **.exe$$ &

stoptest:
	killall web_server
	rm -rf web/wavecat.exe

clean: thirdpartyclean serverclean
	rm -rf wavecat.exe
	find . -name '*.o' -print0 | xargs -0 rm -f
	find . -name '*~'  -print0 | xargs -0 rm -f

.PHONY = clean
