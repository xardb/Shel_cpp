# Компилятор и флаги
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = kubsh
PACKAGE_NAME = kubsh_1.0-1
DEB_PACKAGE = $(PACKAGE_NAME).deb

# Исходные файлы
SOURCES = main.cpp
HEADERS = 

# Основная цель по умолчанию
all: $(TARGET)

# Компиляция шелла
$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

# Сборка deb пакета
deb: $(TARGET)
	@echo "Сборка DEB пакета..."
	./build-deb.sh

# Очистка
clean:
	rm -f $(TARGET)
	rm -rf $(PACKAGE_NAME)
	rm -f *.deb

# Установка в систему
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# Запуск тестов
test: $(TARGET)
	@echo "Запуск тестов..."
	docker run -v $(PWD)/$(TARGET):/usr/local/bin/$(TARGET) tyvik/kubsh_test:master

.PHONY: all clean install deb test
