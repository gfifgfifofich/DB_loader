# Usage
в поле драйвера вставить название любого драйвера из ODBC data sources(32bit), в поле названия базы данных: ip:port/DatabaseName 
ввести пользователя/пароль и подключить базу данных
выбрав драйвер 'LOCAL_SQLITE_DB' подключение произойдёт с локальным файлом SQLiteDB.db - база данных SQLite, в которую есть опция сохранять выгрузки


# Shortcuts
CTRL + E - запустить скрипт
CTRL + S - сохранить таблицу в файл
CTRL + O - открыть файл/папку (файл - excel/filename.xlsx, папка - excel/)
CTRL + N - открыть новое окно (лучше запускать исполняемый файл)
CTRL + SPACE / Tab - автоподстановка
CTRL + I - окно для множественного запуска скрипта с разными параметрами (Итерирование/цикл)

# Misc
При запуске скрипта база данных переподключается автоматически

Для подключения к базе данных Postgre и правильной автоподстановки/потсветки требуется включить соответствующий флаг в интерфейсе.

userdata.txt - вся информация по базам данных и пользователе
{Имя базы данных}.txt - создаётся при подключении к бд, хранит структуру бд. Если добавились таблицы/ появился доступ - удалить файл.

user_get_tables_columns_script.sql - скрипт, за счет которого достаётся вся структура бд для автоподстановки и подсветки 
Можно редактировать,подойдёт любой скрипт, выдающий lower(имя таблицы),lower(имя столбца).

при запуске скрипта, выполняется либо часть от ; до ; , либо выделенная область

sqlBackup - хранит каждый запуск sql по дате

stock.sql - Открывается при запуске программы, хранит последний запущенный скрипт

тестировались с Oracle, Postgres и SQLite 

Импорт из .csv в sqlite - https://github.com/pawelsalawa/sqlitestudio

# Build
```bash
  mkdir build
  cd ./build
  cmake ../
  cmake --build ./ -j 6
```
Либо открыть CMakeLists.txt в QTCreator, и собрать оттуда

