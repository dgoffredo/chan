BUILD_TYPE ?= Release

SOURCES := $(shell find . -name '*.cpp')
OBJECTS := $(SOURCES:.cpp=.o)

# Note for the unaccustomed: CPPFLAGS are for the C pre-processor, while
# CXXFLAGS are for the C++ compiler.  In general, CPP -> C pre-processor, and
# CXX -> C++ compiler.  Except when they don't (e.g. `.cpp` files are for the
# C++ compiler, though only after having gone through the C pre-processor...).
CPPFLAGS += -I src/
ifeq ($(BUILD_TYPE), Release)
     CPPFLAGS += -DNDEBUG
endif

# Variadic macros (e.g. `#define FOO(a, b, ...)`) are technically not part of
# C++98, but since every C++98/03 compiler I've seen in the last 20 years
# implicitly supports C99, which does have variadic macros, I suppress the
# pedantic warning about them here. This way, you can compile the code using
# any compiler I know of (including Solaris CC and IBM xlc++ -- VC6 doesn't
# count because it's Windows).
WARNINGFLAGS += -Wall -Wextra -pedantic -Werror -Wno-variadic-macros
ifeq ($(BUILD_TYPE), Release)
    OPTIMIZATIONFLAGS += -O3 -flto
else
    OPTIMIZATIONFLAGS += -Og
endif
DEBUGFLAGS += -g
CXXFLAGS += $(WARNINGFLAGS) $(OPTIMIZATIONFLAGS) $(DEBUGFLAGS)

# Put the `.o` files into one `.a` file.  The rule by which the `.o` files are
# made is assumed by `make`, and so does not appear in this makefile.
libchan.a: $(OBJECTS)
	ar rcs libchan.a $(OBJECTS)

.PHONY: clean
clean:
	find src/ -type f \( -name '*.d' -o -name '*.o' -o -name '*.a' \) \
	          -exec rm {} \;

# Use the `-MM` option of the C++ compiler to produce makefile dependencies
# corresponding to the headers included (even transitively) by each C++ file.
# The `sed` command then adds the resulting `.d` file as an additional target,
# so if any of the headers change, the makefile (`.d` file) will be remade
# (along with the `.o`).
%.d: %.cpp
	$(CXX) -MM $(CPPFLAGS) $< | sed '1 s,^.*:,$@ $(patsubst %.d,%.o,$@): ,' >$@

# Include all of the `.d` files, which are makefiles containing the rules that
# say how `.o` files depend on `.h` and `.cpp` files.
-include $(SOURCES:.cpp=.d)
