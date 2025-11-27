//
// Created by xardb on 11/27/25.
//

#include "disk_utils.hpp"
#include <pthread.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <fstream>
#include <chrono>
#include <fcntl.h>
#include <sys/types.h>


using namespace std;

//тут мы накладываем С-структуру на байтовый массив из сектора, чтоб не ковыряться в байтах вручную, а штоб они были похожи на нормальные переменные
//Прагма pack(1) говорит компилятору чтоб тот упаковал структуру без выравнивания, без вставок пустых байтов, за счет вставок и выравнивнивания растет скорость , но ломается спецификация GPT
    #pragma pack(push,1) //push - сохраняет старое состояние упаковки
    struct GPTHeader {
        char signature[8];     // "EFI PART", если они есть - диск GPT
        uint32_t revision; // Версия gpt 00 00 01 00 = 1.0
        uint32_t headerSize; // размер структуры заголовка
        uint32_t headerCRC32; // контрольная сумма , чтоб понять не битый ли у нас заголовок
        uint32_t reserved; // резервация, если там не 0, то это не гпт или оно поломалось, иначе говоря контроль целостности проводим
        uint64_t currentLBA; // где находится этот заголовок (надо LBA1)
        uint64_t backupLBA; // Резервный(обычно последний сектор диска)

        //Диапазон в котором разрешено создавать разделы
        uint64_t firstUsableLBA;
        uint64_t lastUsableLBA;

        unsigned char diskGUID[16]; // ну тут всё ясна, GUID — статистически уникальный 128-битный идентификатор
        uint64_t partEntryLBA; // LBA, где начинается массив записей ГПТ разделов(обычно второй сектор)
        uint32_t numPartEntries; //сколько записей в таблице(обычно 128)
        uint32_t partEntrySize;  // размер одной записи
        uint32_t partArrayCRC32; //crc32 всего раздела(контрольная сумма)
    };
//Каждый раздел GPT - 128 байтная стуктура
    struct GPTEntry {
        unsigned char typeGUID[16]; // тип раздела(Linux fs, Efi system partition, etc)
        unsigned char partGUID[16]; // уникальный гуид каждого раздела

        uint64_t firstLBA;
        uint64_t lastLBA;

        uint64_t attrs; // флаги атрибутов( hidden, readonly, required, bootable)
        uint16_t name[36]; // имя раздела в UTF16
    };
    #pragma pack(pop) // поп - возвращает упаковку назад

//пока разбирал этот код, понравился итог чатгпт, вставлю сюда
//Это делает проверку GPT нормальной, профессиональной, а не «смотрим магическую строку и молимся».
//Определение: LBA - Logical Block Address - просто номер сектора на диске
// LBA0 - MBR\Protective MBR, LBA1 - GPT Header, LBA2 - GPT Partition Table, LBAX - остальная таблица разделов, именно поэтому currentLBA должен быть 1, а backup - последний сектор



//Функция позволяет выводить прочитанный сектор в удобном hexdump формате(00 11 22 33....)
    void dump_sector_hex(const unsigned char* buf) { // Принимаем массив байтов
        for (int i = 0; i < 512; i++) { // Проходим по каждому байту сектора
            printf("%02X ", buf[i]); //выводим байт в шестандцатеричном формате, (%02X - два символа, с ведущим нулем
            if ((i+1) % 16 == 0) printf("\n"); //перенос строки каждые 16 байт
        }
    }

    bool read_gpt(const string& device) {

        //открываем устройство
        int fd = open(device.c_str(), O_RDONLY);
        if (fd < 0) {
            cerr << "Не удалось открыть " << device << endl;
            return false;
        }

        unsigned char buf[512]; // читаем сюда сектора

        // Переходим на LBA1  и читаем GPT Header, если  != 512 - образ обрезан
        lseek(fd, 512, SEEK_SET);
        //offset - текущая позиция чтения файла. lseek ставит эту позицию в 512 байтов от начала, иначе говоря тупо переходим из LBA0 в LBA1
        if (read(fd, buf, 512) != 512) {
            cerr << "Ошибка чтения GPT Header" << endl;
            close(fd);
            return false;
        }

        //отладочная печать RAW байтов заголовка, можно увидеть сигнатуру, CRC, GUID, offsets
        cout << "\n=== RAW LBA1 (GPT Header) ===\n";
        dump_sector_hex(buf);

        GPTHeader hdr; // копируем байты в структуру, благодаря прагме стуктура не съедет
        memcpy(&hdr, buf, sizeof(GPTHeader)); // memcpy копирует байты

        // memcmp - побайтовое сравнение массивов, чтоб понять действительно ли header - GPT
        if (memcmp(hdr.signature, "EFI PART", 8) != 0) {
            cout << "\nGPT сигнатура отсутствует\n";
            //сигнатура - первые 8 байт в заголовке
            close(fd);
            return false;
        }
        //hdr - локальная переменная которая содержит разобранные поля GPTheader
        cout << "\nGPT обнаружен\n";
        cout << "  Кол-во разделов: " << hdr.numPartEntries << endl;
        cout << "  Размер записи:   " << hdr.partEntrySize << " байт" << endl;

        // Читаем первые 3 записи GPT (LBA 2,3,4)
        for (int i = 0; i < 3; i++) {
            uint64_t lba = hdr.partEntryLBA + i;
            lseek(fd, lba * 512, SEEK_SET);

            if (read(fd, buf, 512) != 512) break;

            //печатаем raw hex байты каждой записи
            cout << "\n=== RAW LBA" << lba << " (GPT Entry Block #" << (i+1) << ") ===\n";
            dump_sector_hex(buf);

            GPTEntry e; // это наша стурктура одного раздела
            memcpy(&e, buf, sizeof(GPTEntry)); // копируем в нее данные

            // эт просто проверочка на пустоту
            bool notEmpty = true;
            for (int k = 0; k < 16; k++)
                if (e.typeGUID[k] != 0) notEmpty = false;

            if (notEmpty) continue;
            //если запись есть - выводим содержимое
            cout << "\nРаздел #" << (i+1) << endl;
            cout << "  First LBA: " << e.firstLBA << endl;
            cout << "  Last  LBA: " << e.lastLBA << endl;

            // Имя
            cout << "  Name: ";
            for (int j = 0; j < 36; j++) {
                char c = e.name[j] & 0xFF;
                if (c == 0) break;
                cout << c;
            }
            cout << endl;
        }

        close(fd);
        return true;
    }


// эта функция открывает диск (например /dev/sda я еще узнал, что можно создавать через fdisk тестовые), читает первые (512 байт) и проверяет:
//есть ли мбр-сигнатура 55 AA
//какой загрузочный флаг у первого раздела
//ыводит первые 16 байт первой записи раздела
    void check_mbr(const string& path) {
        cout << "=== Проверка MBR =====\n";

        int fd = open(path.c_str(), O_RDONLY); // открыли файл устройства в readonly
        if (fd < 0) {
            cout << "Не удалось открыть " << path << endl;
            return;
        }

        unsigned char mbr[512]; //сделали буфер размером в один сектор
        if (read(fd, mbr, 512) != 512) {
            cout << "Не удалось прочитать сектор" << endl;
            close(fd);
            return;
        }
        close(fd);

        // MBR Signature
        if (mbr[510] == 0x55 && mbr[511] == 0xAA) {
            cout << "MBR signature OK (55 AA)\n";
        } else {
            cout << "MBR signature NOT FOUND\n";
            return;
        }

        // Boot flag
        unsigned char boot_flag = mbr[446];

        cout << "Boot flag raw byte = 0x" << hex << (int)boot_flag << dec << endl;

        if (boot_flag == 0x80) {
            cout << "Первый раздел: загрузочный (0x80 — active)\n";
        } else if (boot_flag == 0x00) {
            cout << "Первый раздел: не загрузочный (0x00 — флаг не установлен)\n";
        } else {
            cout << "Первый раздел: какой то странный, не стандартный флаг";
        }

        // Print entry bytes
        cout << "Первые 16 байт записи первого раздела:\n  ";
        for (int i = 446; i < 462; i++)
            printf("%02X ", mbr[i]);
        cout << "\n";
    }
