CXX_FLAGS=-Wextra -Wall -pedantic -std=c++17 -g -O2
CXX_INCLUDES=-I include
CXX_HEADERS=$(wildcard include/*.hpp)

all: flower shared

shared: shared/file_provider.so shared/interface_provider.so

shared/file_provider.so: src/shared/file_provider.cpp
	$(CXX) -shared -fPIC $(CXX_FLAGS) $(CXX_INCLUDES) -lpcap $< -o $@

shared/interface_provider.so: src/shared/interface_provider.cpp
	$(CXX) -shared -fPIC $(CXX_FLAGS) $(CXX_INCLUDES) -lpcap $< -o $@

flower: src/main.cpp $(CXX_HEADERS)
	$(CXX) $(CXX_FLAGS) $(CXX_INCLUDES) -ldl $< -o $@

clean:
	rm flower
