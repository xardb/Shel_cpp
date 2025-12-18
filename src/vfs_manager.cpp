#define FUSE_USE_VERSION 35

#include <fuse3/fuse.h>
#include <pthread.h>
#include <unistd.h>

#include "vfs.hpp"  // здесь объявлены users_operations и init_users_operations()

void* fuse_thread_function(void* arg) {
    (void)arg;

    // Инициализируем структуру операций, без этой штуки фьюзе не знает как работать с нашей VFS
    init_users_operations();


    // //это необязательная штука, просто там какой то мусор при ручном запуске выводился, решили выпилить его на всякий
    // int devnull = open("/dev/null", O_WRONLY);
    // int olderr = dup(STDERR_FILENO);
    // dup2(devnull, STDERR_FILENO);
    // close(devnull);


    // Аргументы для fuse_main
    const char* fuse_argv[] = {
        "vfs_users",
        "-f", // foreground (требуется для корректной работы в потоке)
        "-omax_idle_threads=10000",
        "-odefault_permissions",
        "-oauto_unmount",
        "/opt/users"               // точка монтирования
    };
    //vfs_users -f -odefault_permissions -oauto_unmount /opt/users вот такую команду вводим
    //-f не даёт уйти в фон
    //default_permissions - ядро само проверяет unix права
    //auto unmount - автоматическое отмонтирование при завершении
    // /opt/users - хардкодим путь для тестов
    int fuse_argc = sizeof(fuse_argv) / sizeof(fuse_argv[0]);

    // Запуск FUSE (блокирующий вызов)
    fuse_main(fuse_argc, (char**)fuse_argv, &users_operations, NULL);
    //fuse_main - это основная чудо-функция, которая
    // парсит аргументы, создает сессию, монтирует fs, держит fs активной, всё держится на одной этой функции
    return NULL;
}

void fuse_start() {
    pthread_t fuse_thread;
    pthread_create(&fuse_thread, NULL, fuse_thread_function, NULL);
    // Можно pthread_detach(fuse_thread) если не нужен join
}


