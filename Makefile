#
# 2010/12/16 by D. Grant Starkweather
#

MVPVERSION = 0.0.0

HFLS	= mvptree.h
OBJS	= mvptree.o

CC	= cc

CFLAGS	= -g -O3 -I. $(DEFINES)

#edit these lines to reflect the location of the pHash header files (pHash.h)
CPPFLAGS = -pthread -I /usr/local/include

#location of phash-config.h
#CPPFLAGS += -I /usr/local/include

LDFLAGS	=
RANLIB	= ranlib

DESTDIR	= /usr/local
TEST	= testmvp
TEST2	= testmvp2
TEST3   = imget
UTIL1	= image_hash_add
UTIL2	= image_hash_query

LIBRARY	= libmvptree.a

DEPS_LIBS = -lm
PHASH_LIBS = -L/usr/local/lib -lpHash


#uncomment if you are using prng library for random number generation
#DEPS_LIBS += /usr/local/lib/libprng.a


all : $(TEST) $(TEST2)

clean :
	rm -f a.out core *.o *.t
	rm -f $(LIBRARY) $(UTIL) $(TEST) $(TEST2) $(TEST3)

install : $(HFLS) $(LIBRARY) 
	install -c -m 444 $(HFLS) $(DESTDIR)/include
	install -c -m 444 $(LIBRARY) $(DESTDIR)/lib
	$(RANLIB) $(DESTDIR)/lib/$(LIBRARY)

$(LIBRARY) : $(OBJS)
	ar cr $(LIBRARY) $?
	$(RANLIB) $@

imget : $(TEST3)

utils : $(UTIL1) $(UTIL2)

tests : $(TEST) $(TEST2) $(TEST3)

$(TEST) : $(LIBRARY) $(TEST).o 
	rm -f $@
	$(CC) $(CFLAGS) $(LDFLAGS) $(TEST).o $(LIBRARY) $(DEPS_LIBS)
	mv a.out $@

$(TEST2): $(LIBRARY) $(TEST2).o
	rm -f $@
	$(CC) $(CFLAGS) $(LDFLAGS) $(TEST2).o $(LIBRARY) $(DEPS_LIBS)
	mv a.out $@

$(TEST3): $(LIBRARY) $(TEST3).o
	rm -f $@
	g++ $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TEST3).o $(LIBRARY) $(DEPS_LIBS) $(PHASH_LIBS)
	mv a.out $@

.c.o :
	rm -f $@
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST3).o : 
	rm -f $@
	g++ $(CFLAGS) $(CPPFLAGS) -c $(TEST3).cpp -o $@

$(UTIL1): $(LIBRARY) $(UTIL1).o
	rm -f $@
	g++ $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(UTIL1).o $(LIBRARY) $(DEPS_LIBS) $(PHASH_LIBS)
	mv a.out $@

$(UTIL2): $(LIBRARY) $(UTIL2).o
	rm -f $@
	g++ $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(UTIL2).o $(LIBRARY) $(DEPS_LIBS) $(PHASH_LIBS)
	mv a.out $@
