# Компилятор и флаги
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Wno-missing-field-initializers
FUSE_FLAGS = -I/usr/include/fuse3 -lfuse3 -L/usr/lib/x86_64-linux-gnu
TARGET = kubsh

# Docker конфигурация
IMAGE := ghcr.io/xardb/kubshfuse:master
DOCKER_FLAGS := --device /dev/fuse --cap-add SYS_ADMIN --security-opt apparmor:unconfined

# Версия пакета
VERSION = 1.0.0
PACKAGE_NAME = kubsh
BUILD_DIR = build
DEB_DIR = $(BUILD_DIR)/$(PACKAGE_NAME)_$(VERSION)_amd64
DEB_FILE := $(PACKAGE_NAME)_$(VERSION)_amd64.deb

# Исходные файлы
SRCS = \
    src/main.cpp \
    src/disk_utils.cpp \
    src/arg_parser.cpp \
    src/command_path.cpp \
    src/builtin_commands.cpp \
    src/history.cpp \
    src/executor.cpp

OBJS = $(SRCS:.cpp=.o)

# Основные цели
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(FUSE_FLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(FUSE_FLAGS) -c $< -o $@

# Запуск шелла
run: $(TARGET)
	./$(TARGET)

# Подготовка структуры для deb-пакета
prepare-deb: $(TARGET)
	@echo "Подготовка структуры для deb-пакета..."
	@mkdir -p $(DEB_DIR)/DEBIAN
	@mkdir -p $(DEB_DIR)/usr/local/bin
	@cp $(TARGET) $(DEB_DIR)/usr/local/bin/
	@chmod +x $(DEB_DIR)/usr/local/bin/$(TARGET)

	@echo "Создание control файла..."
	@echo "Package: $(PACKAGE_NAME)" > $(DEB_DIR)/DEBIAN/control
	@echo "Version: $(VERSION)" >> $(DEB_DIR)/DEBIAN/control
	@echo "Section: utils" >> $(DEB_DIR)/DEBIAN/control
	@echo "Priority: optional" >> $(DEB_DIR)/DEBIAN/control
	@echo "Architecture: amd64" >> $(DEB_DIR)/DEBIAN/control
	@echo "Maintainer: Your Name <your.email@example.com>" >> $(DEB_DIR)/DEBIAN/control
	@echo "Description: Simple custom shell" >> $(DEB_DIR)/DEBIAN/control
	@echo " A simple custom shell implementation for learning purposes." >> $(DEB_DIR)/DEBIAN/control

	@echo "Создание postinst скрипта..."
	@echo "#!/bin/bash" > $(DEB_DIR)/DEBIAN/postinst
	@echo "set -e" >> $(DEB_DIR)/DEBIAN/postinst
	@echo "# Создаем точку монтирования VFS" >> $(DEB_DIR)/DEBIAN/postinst
	@echo "mkdir -p /opt/users" >> $(DEB_DIR)/DEBIAN/postinst
	@echo "chmod 755 /opt/users" >> $(DEB_DIR)/DEBIAN/postinst
	@echo "exit 0" >> $(DEB_DIR)/DEBIAN/postinst
	@chmod +x $(DEB_DIR)/DEBIAN/postinst

# Сборка deb-пакета
deb: prepare-deb
	@echo "Сборка deb-пакета..."
	dpkg-deb --build $(DEB_DIR)
	@mv $(BUILD_DIR)/$(PACKAGE_NAME)_$(VERSION)_amd64.deb ./$(DEB_FILE)
	@echo "Пакет создан: $(DEB_FILE)"

# Установка пакета (требует sudo)
install: deb
	sudo dpkg -i $(DEB_FILE)

# Удаление пакета
uninstall:
	sudo dpkg -r $(PACKAGE_NAME)

# Тестирование в Docker контейнере
test: clean deb
	@echo "Запуск теста в Docker контейнере..."
	@echo "Используется файл: $(DEB_FILE)"
	docker run -it \
          -v $(CURDIR)/$(DEB_FILE):/mnt/kubsh.deb \
          $(DOCKER_FLAGS) \
          $(IMAGE)

# Очистка
clean:
	rm -rf $(BUILD_DIR) $(TARGET) *.deb $(OBJS)

# Показать справку
help:
	@echo "Доступные команды:"
	@echo "  make all       - собрать программу"
	@echo "  make deb       - создать deb-пакет"
	@echo "  make install   - установить пакет"
	@echo "  make uninstall - удалить пакет"
	@echo "  make clean     - очистить проект"
	@echo "  make run       - запустить шелл"
	@echo "  make test      - запустить тест в Docker контейнере"
	@echo "  make help      - показать эту справку"

.PHONY: all deb install uninstall clean help prepare-deb run test