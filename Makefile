# Компилятор и флаги
CXX := g++
CXXFLAGS := -O2 -std=c++11

# Имя программы
TARGET := kubsh

# Настройки пакета
PACKAGE_NAME := $(TARGET)
VERSION := 1.0
ARCH := amd64
DEB_FILENAME := kubsh.deb

# Временные директории
BUILD_DIR := deb_build
INSTALL_DIR := $(BUILD_DIR)/usr/local/bin

.PHONY: all clean deb

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

deb: $(TARGET) | $(BUILD_DIR) $(INSTALL_DIR)
	# Копируем бинарник
	cp $(TARGET) $(INSTALL_DIR)/
	
	# Создаем базовую структуру пакета
	mkdir -p $(BUILD_DIR)/DEBIAN
	
	# Генерируем контрольный файл
	@echo "Package: $(PACKAGE_NAME)" > $(BUILD_DIR)/DEBIAN/control
	@echo "Version: $(VERSION)" >> $(BUILD_DIR)/DEBIAN/control
	@echo "Architecture: $(ARCH)" >> $(BUILD_DIR)/DEBIAN/control
	@echo "Maintainer: $(USER)" >> $(BUILD_DIR)/DEBIAN/control
	@echo "Description: Simple shell" >> $(BUILD_DIR)/DEBIAN/control
	
	# Собираем пакет с фиксированным именем
	dpkg-deb --build $(BUILD_DIR) $(DEB_FILENAME)

$(BUILD_DIR) $(INSTALL_DIR):
	mkdir -p $@

clean:
	rm -rf $(TARGET) $(BUILD_DIR) $(DEB_FILENAME)
