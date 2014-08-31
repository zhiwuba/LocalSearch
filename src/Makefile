DIR_SRC=./
DIR_3RD=../3rd/
DIR_MYSQL_LIB=/usr/lib64/mysql/
TARGET=search


CPPS=$(wildcard $(DIR_SRC)*.cpp)
CPPS+=$(wildcard $(DIR_SRC)/pugixml/*.cpp)
OBJS=$(patsubst %.cpp,%.o,$(CPPS))

CFLAGS=-O -DLINUX
CFLAGS+=-I $(DIR_3RD)libscws/include
CFLAGS+=-I $(DIR_3RD)

CC=g++
LIB=-L $(DIR_3RD)libscws/lib/ -lscws -pthread
LIB+=-L $(DIR_MYSQL_LIB) -lmysqlclient

all:$(TARGET)
$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIB) -o $(TARGET) 

$(OBJS):%.o:%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJS) $(TARGET)


