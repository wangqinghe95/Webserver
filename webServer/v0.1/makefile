DIR_OBJ=./obj
DIR_SRC=.
DIR_BIN=./bin
DIR_INC=./

SRC=$(wildcard ${DIR_SRC}/*.cpp)
INC=$(wildcard $(DIR_SRC)/*.hpp)
OBJ=$(patsubst %.cpp, ${DIR_OBJ}/%.o, $(notdir ${SRC}))

TARGET=webserver
BIN_TARGET=$(DIR_BIN)/$(TARGET)
INCLUDE= -I ${DIR_INC}
LIBS = -lpthread

CFLAGS=-g -Wall

${BIN_TARGET}:${OBJ}
	g++ ${CFLAGS} ${INCLUDE} ${OBJ} ${LIBS} -o $@

${DIR_OBJ}/%.o:${DIR_SRC}/%.cpp
	g++ ${CFLAGS} ${INCLUDE} -c $< -o $@

.PHONY:clean
clean:
	rm -rf ${DIR_OBJ}/*.o ${BIN_TARGET}


