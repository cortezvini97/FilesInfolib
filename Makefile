# Defina o compilador e os flags de compilação
CC = gcc
CFLAGS = -fPIC -Iinclude -I/usr/include/cjson
LDFLAGS = -shared -lcjson

# Diretórios de destino
BUILD_DIR = build
DIST_DIR = dist
TEST_DIR = tests
INCLUDE_DIR = /usr/include/Fileinfolib
LIB_DIR = /usr/lib/Fileinfolib
LD_CONFIG_PATH = /etc/ld.so.conf.d/Fileinfolib.conf

# Nome do arquivo de saída
TARGET = $(DIST_DIR)/libfileinfo.so
TEST_EXEC = $(TEST_DIR)/main

# Lista de arquivos fonte para a biblioteca
LIB_SRCS = src/fileinfo.c
LIB_OBJS = $(patsubst src/%.c, $(BUILD_DIR)/%.o, $(LIB_SRCS))

# Lista de arquivos fonte para o executável de teste
TEST_SRCS = src/main.c src/fileinfo.c
TEST_OBJS = $(patsubst src/%.c, $(BUILD_DIR)/%.o, $(TEST_SRCS))

# Regra padrão
all: $(TARGET)

# Criação dos diretórios de destino, se necessário
$(BUILD_DIR) $(DIST_DIR) $(TEST_DIR):
	mkdir -p $@

# Regra para criar a biblioteca compartilhada
$(TARGET): $(LIB_OBJS) | $(DIST_DIR)
	$(CC) $(LDFLAGS) -o $@ $(LIB_OBJS)

# Regra para compilar arquivos .c em .o e colocar em build/
$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para compilar e gerar o executável de teste
$(TEST_EXEC): $(BUILD_DIR)/main.o $(BUILD_DIR)/fileinfo.o | $(TEST_DIR)
	$(CC) -o $@ $(BUILD_DIR)/main.o $(BUILD_DIR)/fileinfo.o

# Regra para compilar main.c em main.o
$(BUILD_DIR)/main.o: src/main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c src/main.c -o $@

# Regra para compilar fileinfo.c em fileinfo.o
$(BUILD_DIR)/fileinfo.o: src/fileinfo.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c src/fileinfo.c -o $@

# Regra para limpar os arquivos gerados
clean:
	rm -f $(LIB_OBJS) $(TARGET)
	rm -f $(TEST_OBJS) $(TEST_EXEC)
	rmdir $(BUILD_DIR) 2>/dev/null || true
	rmdir $(TEST_DIR) 2>/dev/null || true
	rm -rf $(DIST_DIR)

# Regra para compilar e executar o executável de teste
test: $(TEST_EXEC)
	$(TEST_EXEC)

install: $(TARGET)
	make unistall
	mkdir -p $(INCLUDE_DIR)
	mkdir -p $(LIB_DIR)
	cp include/fileinfo.h $(INCLUDE_DIR)/
	cp $(TARGET) $(LIB_DIR)/
	echo "$(LIB_DIR)" > $(LD_CONFIG_PATH)
	ldconfig

unistall:
	rm -f $(INCLUDE_DIR)/fileinfo.h
	rm -f $(LIB_DIR)/libfileinfo.so
	rm -f $(LD_CONFIG_PATH)
	ldconfig

.PHONY: all clean test install uninstall