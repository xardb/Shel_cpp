#!/bin/bash

# Настройки
PACKAGE_NAME="kubsh_1.0-1"
BINARY_NAME="kubsh"
MAINTAINER="xardb <vlad96mir@bk.ru>"

# Создаем структуру директорий
echo "Создаем структуру пакета..."
rm -rf $PACKAGE_NAME
mkdir -p $PACKAGE_NAME/DEBIAN
mkdir -p $PACKAGE_NAME/usr/local/bin

# Копируем бинарник
echo "Копируем бинарник..."
cp $BINARY_NAME $PACKAGE_NAME/usr/local/bin/
chmod +x $PACKAGE_NAME/usr/local/bin/$BINARY_NAME

# Создаем файл control
echo "Создаем control файл..."
cat > $PACKAGE_NAME/DEBIAN/control << EOF
Package: kubsh
Version: 1.0-1
Section: utils
Priority: optional
Architecture: amd64
Depends: libc6
Maintainer: $MAINTAINER
Description: Simple C++ shell with basic commands
 A learning project shell implementing:
 - Built-in commands (\\q, echo, \\e)
 - External command execution
 - Command history
 - PATH lookup
Homepage: https://github.com/xardb/Shel_cpp
EOF

# Собираем пакет
echo "Собираем DEB пакет..."
dpkg-deb --build $PACKAGE_NAME

# Проверяем что пакет создан
if [ -f "$PACKAGE_NAME.deb" ]; then
    echo "✅ Пакет успешно создан: $PACKAGE_NAME.deb"
    echo "📦 Информация о пакете:"
    dpkg -I $PACKAGE_NAME.deb
else
    echo "❌ Ошибка при создании пакета"
    exit 1
fi
