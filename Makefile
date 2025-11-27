# Компилятор и флаги
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Wno-missing-field-initializers
FUSE_FLAGS = -I/usr/include/fuse3 -lfuse3 -L/usr/lib/x86_64-linux-gnu
TARGET = kubsh

# Docker конфигурация
IMAGE := ghcr.io/xardb/kubshfuse:master
DOCKER_FLAGS := --device /dev/fuse --cap-add SYS_ADMIN --security-opt apparmor:unconfined

# Пакет
PACKAGE_NAME = kubsh
BUILD_DIR = build
DEB_DIR = $(BUILD_DIR)/$(PACKAGE_NAME)
DEB_FILE := $(PACKAGE_NAME).deb

# Исходные файлы
SRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:.cpp=.o)

# Основные цели
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(FUSE_FLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(FUSE_FLAGS) -c $< -o $@

# Сборка deb-пакета
deb: $(TARGET)
	@echo "Создание deb-пакета..."
	@mkdir -p $(DEB_DIR)/DEBIAN
	@mkdir -p $(DEB_DIR)/usr/local/bin
	@cp $(TARGET) $(DEB_DIR)/usr/local/bin/
	@chmod +x $(DEB_DIR)/usr/local/bin/$(TARGET)

	@echo "Package: $(PACKAGE_NAME)" > $(DEB_DIR)/DEBIAN/control
	@echo "Version: 1.0.0" >> $(DEB_DIR)/DEBIAN/control
	@echo "Section: utils" >> $(DEB_DIR)/DEBIAN/control
	@echo "Priority: optional" >> $(DEB_DIR)/DEBIAN/control
	@echo "Architecture: amd64" >> $(DEB_DIR)/DEBIAN/control
	@echo "Maintainer: xardb" >> $(DEB_DIR)/DEBIAN/control
	@echo "Description: Simple custom shell" >> $(DEB_DIR)/DEBIAN/control

	dpkg-deb --build $(DEB_DIR) $(DEB_FILE)
	@echo "Пакет создан: $(DEB_FILE)"

# Тестирование в Docker
test: deb
	docker run -it -v $(CURDIR)/$(DEB_FILE):/mnt/kubsh.deb $(DOCKER_FLAGS) $(IMAGE)

# Запуск шелла
run: $(TARGET)
	sudo ./$(TARGET)

# Очистка
clean:
	rm -rf $(BUILD_DIR) $(TARGET) *.deb $(OBJS)

.PHONY: all deb test clean run