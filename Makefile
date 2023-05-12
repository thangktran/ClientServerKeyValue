IDIR =./
ODIR=obj

CC=g++
CXXFLAGS=-I$(IDIR) -std=c++20 -g
LIBS= -pthread -lrt
TEST_CXXFLAGS=-I$(IDIR)/third_party

_DEPS = HashTable.hpp IpcEndpoint.hpp ComProtocol.hpp
_OBJ = HashTable.o IpcEndpoint.o
_TEST_OBJ = main.o HashTableTest.o IpcEndpointTest.o

all: mkdir KvServer KvClient unittests
.PHONY: all KvServer KvClient unittests clean mkdir

DEPS = $(patsubst %,$(IDIR)/include/%,$(_DEPS))
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))
TEST_OBJ = $(patsubst %,$(ODIR)/tests/%,$(_TEST_OBJ))

$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CXXFLAGS)

$(ODIR)/%.o: src/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CXXFLAGS)

$(ODIR)/tests/%.o: tests/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CXXFLAGS) $(TEST_CXXFLAGS)

KvServer: $(OBJ) obj/KvServer.o
	$(CC) -o $@ $^ $(CXXFLAGS) $(LIBS)

KvClient: $(OBJ) obj/KvClient.o
	$(CC) -o $@ $^ $(CXXFLAGS) $(LIBS) 

unittests: $(OBJ) $(TEST_OBJ)
	$(CC) -o $@ $^ $(CXXFLAGS) $(LIBS)

MKDIR_P = mkdir -p
mkdir:
	${MKDIR_P} ${ODIR}
	${MKDIR_P} ${ODIR}/tests

clean:
	rm -f ./KvServer ./KvClient ./unittests
	rm -rf ${ODIR}