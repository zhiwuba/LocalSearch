DIR_SRC=./
TARGET=search


CPPS=$(wildcard $(DIR_SRC)*.cpp)
CPPS+=$(wildcard $(DIR_SRC)/pugixml/*.cpp)
OBJS=$(patsubst %.cpp,%.o,$(CPPS))

CFLAGS=-O 

CC=g++
LIB=

all:$(TARGET)
$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIB) -o $(TARGET) 

$(OBJS):%.o:%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJS) $(TARGET)


