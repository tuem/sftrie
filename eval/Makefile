# sftrie
# https://github.com/tuem/sftrie
#
# Copyright 2017 Takashi Uemura
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

VALIDATE_SET_BIN = validate_set
VALIDATE_MAP_BIN = validate_map
CONSTRUCT_SET_BIN = construct_set
CONSTRUCT_MAP_BIN = construct_map
BENCHMARK_SET_BIN = benchmark_set
BENCHMARK_MAP_BIN = benchmark_map
BINS = $(VALIDATE_SET_BIN) $(VALIDATE_MAP_BIN) $(CONSTRUCT_SET_BIN) $(CONSTRUCT_MAP_BIN) $(BENCHMARK_SET_BIN) $(BENCHMARK_MAP_BIN)

all: $(BINS)


CXX := g++
CXXFLAGS := -Wall -Wextra -O3 -std=c++11 -I../.. -isystem include -isystem include/cmdline -isystem include/json

debug: CXXFLAGS += -DDEBUG -g
debug: all

naive: clean all
naive: CXXFLAGS += -DSFTRIE_SET_USE_NAIVE
naive: CXXFLAGS += -DSFTRIE_MAP_USE_NAIVE
naive: VALIDATE_SET_BIN := $(VALIDATE_SET_BIN)_naive
naive: VALIDATE_MAP_BIN := $(VALIDATE_MAP_BIN)_naive
naive: CONSTRUCT_SET_BIN := $(CONSTRUCT_SET_BIN)_naive
naive: CONSTRUCT_MAP_BIN := $(CONSTRUCT_MAP_BIN)_naive
naive: BENCHMARK_SET_BIN := $(BENCHMARK_SET_BIN)_naive
naive: BENCHMARK_MAP_BIN := $(BENCHMARK_MAP_BIN)_naive

basic: clean all
basic: CXXFLAGS += -DSFTRIE_SET_USE_BASIC
basic: CXXFLAGS += -DSFTRIE_MAP_USE_BASIC
basic: VALIDATE_SET_BIN := $(VALIDATE_SET_BIN)_basic
basic: VALIDATE_MAP_BIN := $(VALIDATE_MAP_BIN)_basic
basic: CONSTRUCT_SET_BIN := $(CONSTRUCT_SET_BIN)_basic
basic: CONSTRUCT_MAP_BIN := $(CONSTRUCT_MAP_BIN)_basic
basic: BENCHMARK_SET_BIN := $(BENCHMARK_SET_BIN)_basic
basic: BENCHMARK_MAP_BIN := $(BENCHMARK_MAP_BIN)_basic

tail: clean all
tail: CXXFLAGS += -DSFTRIE_SET_USE_TAIL
tail: CXXFLAGS += -DSFTRIE_MAP_USE_TAIL
tail: VALIDATE_SET_BIN := $(VALIDATE_SET_BIN)_tail
tail: VALIDATE_MAP_BIN := $(VALIDATE_MAP_BIN)_tail
tail: CONSTRUCT_SET_BIN := $(CONSTRUCT_SET_BIN)_tail
tail: CONSTRUCT_MAP_BIN := $(CONSTRUCT_MAP_BIN)_tail
tail: BENCHMARK_SET_BIN := $(BENCHMARK_SET_BIN)_tail
tail: BENCHMARK_MAP_BIN := $(BENCHMARK_MAP_BIN)_tail

decompaction: clean all
decompaction: CXXFLAGS += -DSFTRIE_SET_USE_DECOMPACTION
decompaction: CXXFLAGS += -DSFTRIE_MAP_USE_DECOMPACTION
decompaction: VALIDATE_SET_BIN := $(VALIDATE_SET_BIN)_decompaction
decompaction: VALIDATE_MAP_BIN := $(VALIDATE_MAP_BIN)_decompaction
decompaction: CONSTRUCT_SET_BIN := $(CONSTRUCT_SET_BIN)_decompaction
decompaction: CONSTRUCT_MAP_BIN := $(CONSTRUCT_MAP_BIN)_decompaction
decompaction: BENCHMARK_SET_BIN := $(BENCHMARK_SET_BIN)_decompaction
decompaction: BENCHMARK_MAP_BIN := $(BENCHMARK_MAP_BIN)_decompaction

SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SRCS))
DEPS = $(patsubst %.cpp,%.d,$(SRCS))


$(VALIDATE_SET_BIN): validate_set.o string_util.o
	$(CXX) -o $(VALIDATE_SET_BIN) validate_set.o string_util.o

$(VALIDATE_MAP_BIN): validate_map.o string_util.o
	$(CXX) -o $(VALIDATE_MAP_BIN) validate_map.o string_util.o

$(CONSTRUCT_SET_BIN): construct_set.o string_util.o history.o
	$(CXX) -o $(CONSTRUCT_SET_BIN) construct_set.o string_util.o history.o

$(CONSTRUCT_MAP_BIN): construct_map.o string_util.o history.o
	$(CXX) -o $(CONSTRUCT_MAP_BIN) construct_map.o string_util.o history.o

$(BENCHMARK_SET_BIN): benchmark_set.o string_util.o history.o
	$(CXX) -o $(BENCHMARK_SET_BIN) benchmark_set.o string_util.o history.o

$(BENCHMARK_MAP_BIN): benchmark_map.o string_util.o history.o
	$(CXX) -o $(BENCHMARK_MAP_BIN) benchmark_map.o string_util.o history.o


clean:
	$(RM) $(OBJS) $(DEPS) $(BINS)


.PHONY: clean all


.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

%.d: %.cpp
	@set -e; $(CXX) -MM $(CXXFLAGS) $< \
	| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
	[ -s $@ ] || rm -f $@
-include $(DEPS)
