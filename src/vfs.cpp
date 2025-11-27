#define FUSE_USE_VERSION 35 // версия фьза

#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <pwd.h> // структуры и ф-ции для работы с passwd
#include <sys/types.h>
#include <cerrno>
#include <ctime>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <sys/wait.h>
#include <iostream>

#include "vfs.hpp"

//запуск внешней команды, можно прикрутить сюда executor, но уже пусть так
// Выполняет команду через fork+execvp и ждёт завершения.
// argv должен быть NULL-терминированным.
int run_cmd(const char* cmd, char* const argv[]) {
    pid_t pid = fork();
    if (pid < 0) {
        return -1;
    }

    if (pid == 0) {
        execvp(cmd, argv);
        _exit(127); // если exec не удался
    }

    int status = 0;
    if (waitpid(pid, &status, 0) == -1) {
        return -1;
    }

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) return 0;
    return -1;
}

// Простая проверка валидности shell (заканчивается на sh).
//потому что в реаддир мы перечисляем только пользователей с "валидным шеллом", а не всякие системные учетки
bool valid_shell(struct passwd* pwd)
{
    if (!pwd || !pwd->pw_shell) return false;
    size_t len = strlen(pwd->pw_shell);
    if (len < 2) return false;
    return (strcmp(pwd->pw_shell + len - 2, "sh") == 0);
}


//FUse callback для получения атрибутов файла\дирректории
// path - путь внутри смонтированной файловой системы
//st - заполнить данными(тип, права, владелец, размер, время
//fi - тут не используется, но запрашивается, поэтому сделали заглушку
int users_getattr(const char* path, struct stat* st, struct fuse_file_info* fi) {
    (void) fi; // штука которая игнорирует предупредление компилятора о неиспользуемом параметре fi
    memset(st, 0, sizeof(struct stat));

    time_t now = time(NULL); // заполняем текущим временем
    st->st_atime = st->st_mtime = st->st_ctime = now;
    //я так и не понял зачем оно надо, оно хочет эти поля - мы дадим
    // иначе файл будут считать битым

    // Корень FS
    if (strcmp(path, "/") == 0) {
        st->st_mode = S_IFDIR | 0755; // 0755 значит владелец - rwx, остальные - r-x
        //S_IFDIR -  бит, указывает тип - "дирректория"
        st->st_uid = getuid();
        st->st_gid = getgid();
        st->st_nlink = 2;
        //nlink - число жёстких ссылок: для дирректории минимально 2( точка и родитель)
        return 0;
    }

    //буферы для разбора пути
    char username[256] = {0};
    char filename[256] = {0};

    // попытка распарсить файл: /username/<file>
    if (sscanf(path, "/%255[^/]/%255[^/]", username, filename) == 2) {
        struct passwd* pwd = getpwnam(username);
        if (!pwd) return -ENOENT; // нет такого файла\каталога

        // поддерживаем только три файла: id, home, shell
        if (strcmp(filename, "id") != 0 &&
            strcmp(filename, "home") != 0 &&
            strcmp(filename, "shell") != 0) {
            return -ENOENT;
        }

        // Обычный файл: S_IFREG
        st->st_mode = S_IFREG | 0644; //rw - owner, r-- others
        st->st_uid = pwd->pw_uid;
        st->st_gid = pwd->pw_gid;
        st->st_nlink = 1;

        if (strcmp(filename, "id") == 0) {
            // вводим размер для числа uid
            st->st_size = 16;
        } else if (strcmp(filename, "home") == 0) {
            st->st_size = pwd->pw_dir ? (off_t)strlen(pwd->pw_dir) : 0;
        } else {
            st->st_size = pwd->pw_shell ? (off_t)strlen(pwd->pw_shell) : 0;
        }
        return 0;
    }

    // Директория пользователя: /username
    if (sscanf(path, "/%255[^/]", username) == 1) {
        struct passwd* pwd = getpwnam(username);
        if (!pwd) return -ENOENT;

        st->st_mode = S_IFDIR | 0755;
        st->st_uid = pwd->pw_uid;
        st->st_gid = pwd->pw_gid;
        st->st_nlink = 2;
        return 0;
    }

    return -ENOENT;
}

// readdir: root -> список пользователей, /username -> id,home,shell (только если пользователь есть)
int users_readdir(
    const char* path,
    void* buf,
    fuse_fill_dir_t filler,
    off_t offset,
    struct fuse_file_info* fi,
    enum fuse_readdir_flags flags
) {
    (void) offset;
    (void) fi;
    (void) flags;

    filler(buf, ".", NULL, 0, FUSE_FILL_DIR_PLUS);
    filler(buf, "..", NULL, 0, FUSE_FILL_DIR_PLUS);

    // Корень: перечисляем пользователей из /etc/passwd
    if (strcmp(path, "/") == 0) {
        struct passwd* pwd;
        setpwent();
        while ((pwd = getpwent()) != NULL) {
            if (valid_shell(pwd)) {
                filler(buf, pwd->pw_name, NULL, 0, FUSE_FILL_DIR_PLUS);
            }
        }
        endpwent();
        return 0;
    }

    // Каталог пользователя
    char username[256] = {0};
    if (sscanf(path, "/%255[^/]", username) == 1) {
        struct passwd* pwd = getpwnam(username);
        if (!pwd) return -ENOENT;

        filler(buf, "id", NULL, 0, FUSE_FILL_DIR_PLUS);
        filler(buf, "home", NULL, 0, FUSE_FILL_DIR_PLUS);
        filler(buf, "shell", NULL, 0, FUSE_FILL_DIR_PLUS);
        return 0;
    }

    return -ENOENT;
}

//эта функция возвращает содержимое виртуальных файлов
// id-  текстовый uid
// home - pw_dir
//shell - pw_shell

int users_read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
    (void) fi;

    //Всегла проверяем количество распаршеных(распарсеных?) полей
    char username[256];
    char filename[256];

    std::sscanf(path, "/%255[^/]/%255[^/]", username, filename);

    //стащили запись пользователя
    struct passwd* pwd = getpwnam(username);
    if (!pwd) return -ENOENT;

    char content[256] = {0}; // Буфер для uid, home, shell

    if (std::strcmp(filename, "id") == 0) {
        std::snprintf(content, sizeof(content), "%d", pwd->pw_uid);
    }
    else if (std::strcmp(filename, "home") == 0) {
        std::snprintf(content, sizeof(content), "%s", pwd->pw_dir);
    }
    else if (std::strcmp(filename, "shell") == 0) {
        std::snprintf(content, sizeof(content), "%s", pwd->pw_shell);
    } else {
        return -ENOENT;
    }

    //Длина контента
    size_t len = std::strlen(content);
    // Если offset за пределами - не читаем
    if ((size_t)offset >= len)
        return 0;

    //ограничили размер чтоб не выйти за len
    if (offset + size > len)
        size = len - offset;

    //копируем в буфер, возвращаем количество байт
    std::memcpy(buf, content + offset, size);
    return size;
}


/* Создание "каталога пользователя" вызывает adduser.
     - разрешаем только создание /username (без вложенных путей)
     - если пользователь уже существует — возвращаем -EEXIST
     - иначе запускаем adduser --disabled-password --gecos '' username
     - при успехе возвращаем 0, иначе -EIO
*/
int users_mkdir(const char* path, mode_t mode) {
    (void) mode;

    char username[256] = {0};
    if (sscanf(path, "/%255[^/]", username) != 1) {
        return -EINVAL;
    }
    if (strchr(path + 1, '/') != NULL) {
        return -EPERM;
    }
    struct passwd* pwd = getpwnam(username);
    if (pwd != NULL) return -EEXIST;

    //аргументы для execvp, который запускает adduser
    char* const argv[] = {
        (char*)"adduser",
        (char*)"--disabled-password",
        (char*)"--gecos",
        (char*)"",
        username,
        NULL
    };
// итого: adduser --disabled-password --gecos "" xardb

    if (run_cmd("adduser", argv) == 0) return 0;
    return -EIO;
}

/* Удаление каталога пользователя вызывает userdel --remove.
 Проверяем, что путь корректен и пользователь существует
 Ззапрещаем удалять что-то вроде /user/whatever
*/
int users_rmdir(const char* path) {
    char username[256] = {0};
    if (sscanf(path, "/%255[^/]", username) != 1) {
        return -EINVAL;
    }

    // не позволяем удалять что-либо кроме корневого каталога пользователя
    if (strchr(path + 1, '/') != NULL) {
        return -EPERM;
    }

    struct passwd* pwd = getpwnam(username);
    if (!pwd) return -ENOENT;

    char* const argv[] = {
        (char*)"userdel",
        (char*)"--remove",
        username,
        NULL
    };

    if (run_cmd("userdel", argv) == 0) return 0;
    return -EIO;
}

// Операции FUSE
struct fuse_operations users_operations = {};

void init_users_operations() {
    users_operations.getattr = users_getattr;
    users_operations.readdir = users_readdir;
    users_operations.read = users_read;
    users_operations.mkdir = users_mkdir;
    users_operations.rmdir = users_rmdir;
}
