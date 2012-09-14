ECXX=-Wall
SHARED=../common
V=-DVER="\"$(shell git log -n1 --format=%H HEAD)\"" -DVERDATE="\"$(shell date -u -d@`git log -n1 --format=%at HEAD` +%FT%TZ)\""

all: readb.o common.o
	${CXX} ${CXXFLAGS} ${ECXX} -o readb readb.o ${SHARED}/common.o

%.o: %.cpp
	$(CXX) $(CXXFLAGS) ${ECXX} -c $< -o $@ ${V}

common.o: ${SHARED}/common.cpp
	${CXX} ${CXXFLAGS} -c ${SHARED}/common.cpp -o ${SHARED}/common.o
	
clean:
	rm -f *.o 2>/dev/null