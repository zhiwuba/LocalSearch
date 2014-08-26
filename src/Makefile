DIR_SRC=./
DIR_OBJ=./obj
DIR_BIN=./bin

CPPS=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp,${DIR_OBJ}/%.o,$(notdir ${CPPS}))


all:
	g++ $(CPPS) -o $(OBJS)

clean:
	rm *.o

