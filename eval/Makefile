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

VALIDATE_BIN = validate
BENCHMARK_BIN = benchmark
BINS = $(VALIDATE_BIN) $(BENCHMARK_BIN)

all: $(BINS)


CXX := g++
CXXFLAGS := -Wall -Wextra -O3 -std=c++11 -I..

debug: CXXFLAGS += -DDEBUG -g
debug: all

simple: clean all
simple: CXXFLAGS += -DSFTRIE_USE_SIMPLE
simple: VALIDATE_BIN := $(VALIDATE_BIN)_simple
simple: BENCHMARK_BIN := $(BENCHMARK_BIN)_simple

basic: clean all
basic: CXXFLAGS += -DSFTRIE_USE_BASIC
basic: VALIDATE_BIN := $(VALIDATE_BIN)_basic
basic: BENCHMARK_BIN := $(BENCHMARK_BIN)_basic

tail: clean all
tail: CXXFLAGS += -DSFTRIE_USE_TAIL
tail: VALIDATE_BIN := $(VALIDATE_BIN)_tail
tail: BENCHMARK_BIN := $(BENCHMARK_BIN)_tail

decompaction: clean all
decompaction: CXXFLAGS += -DSFTRIE_USE_DECOMPACTION
decompaction: VALIDATE_BIN := $(VALIDATE_BIN)_decompaction
decompaction: BENCHMARK_BIN := $(BENCHMARK_BIN)_decompaction

SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SRCS))
DEPS = $(patsubst %.cpp,%.d,$(SRCS))


$(VALIDATE_BIN): validate.o string_util.o
	$(CXX) -o $(VALIDATE_BIN) validate.o string_util.o

$(BENCHMARK_BIN): benchmark.o string_util.o history.o
	$(CXX) -o $(BENCHMARK_BIN) benchmark.o string_util.o history.o


clean:
	$(RM) $(OBJS) $(DEPS) $(BINS) $(wildcard validate_*) $(wildcard benchmark_*)


.PHONY: clean all


.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

%.d: %.cpp
	@set -e; $(CXX) -MM $(CXXFLAGS) $< \
	| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
	[ -s $@ ] || rm -f $@
-include $(DEPS)
