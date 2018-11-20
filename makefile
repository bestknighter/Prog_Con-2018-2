#-------------------------------------------------------------
# Assume-se uma distribuição Linux como sistema operacional padrão
#-------------------------------------------------------------

# Nome do executável
EXEC = rodoSim

# Helper para remover nomes duplicados
uniq = $(if $1,$(firstword $1) $(call uniq,$(filter-out $(firstword $1),$1)))

# Compilador
COMPILER = g++

# Comando para remover pastas
RMDIR = rm -rf

# Comando para remover arquivos
RM = rm -f

# Flags para geração automática de dependências
DEP_FLAGS = -MT $@ -MMD -MP -MF $(DEP_PATH)/$*.d

# Flags usadas na fase de compilacao do objeto
INC_FLAGS = -Iinclude -pthread

# Flags usadas na fase de linkagem
LINK_FLAGS = -pthread

# Flags gerais
FLAGS = -std=c++17 -Wall -pedantic -Wextra -fmax-errors=5 -Wno-unused-parameter -Werror=init-self
DFLAGS = -ggdb -O0
RFLAGS = -O3 -mtune=native

# Pastas do projeto
SRC_PATH = src
INC_PATH = include
BIN_PATH = bin
DEP_PATH = dep

#Uma lista de arquivos por extensão:
CPP_FILES = $(wildcard $(SRC_PATH)/*.cpp)
HPP_FILES = $(wildcard $(INC_PATH)/*.hpp)

OBJ_FILES = $(addprefix $(BIN_PATH)/,$(notdir $(CPP_FILES:.cpp=.o)))
DEP_FILES = $(addprefix $(DEP_PATH)/,$(notdir $(CPP_FILES:.cpp=.d)))
DEP_FILES += $(addprefix $(DEP_PATH)/,$(notdir $(HPP_FILES:.hpp=.d)))
DEP_FILES := $(call uniq, $(DEP_FILES))


#-------------------------------------------------------------
# Caso o sistema seja windows
#-------------------------------------------------------------
ifeq ($(OS),Windows_NT)
# Comando para remover um diretório recursivamente
RMDIR= rd /s /q

# Comando para deletar um único arquivo
RM = del

# Path da SDL
DFLAGS += -mconsole
INC_FLAGS := -Iinclude -lpthread
LINK_FLAGS := -lpthread

# Nome do executável
EXEC := $(EXEC).exe

else

UNAME_S := $(shell uname -s)

#-------------------------------------------------------------
# Caso o sistema seja Mac
#-------------------------------------------------------------
ifeq ($(UNAME_S), Darwin)

LINK_FLAGS := -lpthread

endif
endif

#############################################################
#															#
#						Regras								#
#															#
#############################################################

all: $(EXEC)

$(EXEC): $(OBJ_FILES)
	$(COMPILER) -o $@ $^ $(LINK_FLAGS)

$(BIN_PATH)/%.o: $(SRC_PATH)/%.cpp

ifeq ($(OS), Windows_NT)
	@if not exist $(DEP_PATH) @ mkdir $(DEP_PATH)
	@if not exist $(BIN_PATH) @ mkdir $(BIN_PATH)
else
	@mkdir -p $(DEP_PATH) $(BIN_PATH)
endif
	$(COMPILER) $(DEP_FLAGS) -c -o $@ $< $(INC_FLAGS) $(FLAGS)
	
-include $(DEP_FILES)

clean:
	$(RMDIR) $(BIN_PATH) $(DEP_PATH)
	$(RM) $(EXEC)

.PHONY: debug clean release

# Regra pra debug
print-% : ; @echo $* = $($*)

debug: FLAGS += $(DFLAGS)
debug: all

release: FLAGS += $(RFLAGS)
release: all

help:
	@echo.
	@echo Available targets:
	@echo - all:      Builds the standard version (default target)
	@echo - release:  Builds the release version
	@echo - debug:    Builds the debug version
	@echo - help:     Shows this help
	@echo.
