BINARY = simple_calculadora
CXX = g++
salida_test = resultado_test
OBJS = simple_calculadora.o util.o

$(BINARY) : $(OBJS)
	$(CXX) -o $(BINARY) $(OBJS)

all: $(BINARY)

simple_calculadora.o : simple_calculadora.cpp header/simple_calculadora.hpp

util.o : util.cpp util.hpp
.PHONY : clean
clean:
	rm -f $(BINARY) $(OBJS) $(salida_test)