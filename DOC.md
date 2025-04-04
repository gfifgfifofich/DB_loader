# Usage
Драйвера:
```
QOCI - Oracle
QPSQL - Postgre
LOCAL_SQLITE_DB - SQLite, в неё идёт все сейвы.
QODBC - odbc
QODBC_EXCEL - для подключения к excel таблицам и выполнения sql запросов над ними
```
# Sub_syntax
Список команд:
```
SubexecToSilentSqliteTable{ -- {DRIVER} {DB_NAME} {savename}}
SubexecToSilentExcelTable{ -- {DRIVER} {DB_NAME} {savename}}
SubexecToSilentCSV{ -- {DRIVER} {DB_NAME} {savename}}

SubexecToSqliteTable{ -- {DRIVER} {DB_NAME} {savename}}
SubexecToCSV{ -- {DRIVER} {DB_NAME} {savename}}
SubexecToExcelTable{ -- {DRIVER} {DB_NAME} {savename}}

SubexecToUnionAllTable{ -- {DRIVER} {DB_NAME}}
SubexecToMagic{ -- {DRIVER} {DB_NAME}}
SubexecToArray{ -- {DRIVER} {DB_NAME}}

ForLoop { -- {REPLACE_STRING} {from} {to} {step}}
ForLoop { -- {REPLACE_STRING} {value1,value2,value3...}} 
QueryForLoop { -- {REPLACE_STRING} {from} {to} {step}}
QueryForLoop { -- {REPLACE_STRING} {value1,value2,value3...}}
```
Синтаксис любой команды выглядит так:
```
QueryForLoop{
-- {ITER} {01} {12} {1}  // function parameters, shouldn't haave any letters between }{. Every variable is { variable }
--code area
select * from table_ITER
}
```
# Detailed description
## SubexecTo
```
SubexecToSilentSqliteTable{ -- {DRIVER} {DB_NAME} {savename}}
SubexecToSilentExcelTable{ -- {DRIVER} {DB_NAME} {savename}}
SubexecToSilentCSV{ -- {DRIVER} {DB_NAME} {savename}}

SubexecToSqliteTable{ -- {DRIVER} {DB_NAME} {savename}}
SubexecToCSV{ -- {DRIVER} {DB_NAME} {savename}}
SubexecToExcelTable{ -- {DRIVER} {DB_NAME} {savename}}
```
Все команды SubexecToSilent выполняют под запрос, сохраняют в соответствующем виде, не учитываясь основном запросе
```sql
SubexecToSilentSqliteTable
{ 
	-- {QOCI} {127.0.0.1:1521/KTKDB_DB2} {tmp2}
	SELECT * from cool_oracle_table	
}

-- do stuff using result, localy
select * 
from tmp2
cross join tmp
```
Все SubexecTo без Silent возвращяю что-либо обратно скрипт.

Данные команды выводят статус выполнения запроса в скрипт, что позволяет автоматизировать множество запросов в одном, выводя статусы их успешности, размеры выгрузок и прочего
```sql
SubexecToSqliteTable
SubexecToCSV
SubexecToExcelTable
```
Пример
```sql
select 'query 1 ' as query_name, SubexecToExcelTable 
{
-- {LOCAL_SQLITE_DB} {SQLiteDB.db} {q1}
	select * from tmp
}

union all select 'query 2 ' as query_name, SubexecToExcelTable 
{
-- {LOCAL_SQLITE_DB} {SQLiteDB.db} {q2}
	select * from tmp
}
```
result:
```
query 1 	Success	 	8	10
query 2 	Success	 	8	10
```
And files q1.xlsx, q2.xlsx in excel/


## ForLoop
```
ForLoop { -- {REPLACE_STRING} {from} {to} {step}}
ForLoop { -- {REPLACE_STRING} {value1,value2,value3...}} 
QueryForLoop { -- {REPLACE_STRING} {from} {to} {step}}
QueryForLoop { -- {REPLACE_STRING} {value1,value2,value3...}}
```
Разворачивает скрипт, заменяя переменную на соответствующее число в итерации.
Замена происходит в формате первого числа. т.е. диапазон от 1 до 3 выдаст 1 2 3, а диапазон от 01 до 3 выдаст 01 02 03
Script:
```sql
QueryForLoop 
{
-- {str} {01} {5} {1}
	select 'value is str' as column
}
```
Result:
```
value is 01
value is 02
value is 03
value is 04
value is 05
```
Script:
```sql
QueryForLoop 
{
-- {str} {1} {5} {1}
	select 'value is str' as column
}
```
Result:
```
value is 1
value is 2
value is 3
value is 4
value is 5
```
Script:
```sql
QueryForLoop 
{
-- {str} {one,two,   threee    }
	select 'value is str' as column
}
```
Result:
```
value is one
value is two
value is threee
```
## SubexecToArray
```
SubexecToUnionAllTable{ -- {DRIVER} {DB_NAME}}
SubexecToArray{ -- {DRIVER} {DB_NAME}}
SubexecToMagic{ -- {DRIVER} {DB_NAME}}
```
Данные команды выполняют запрос в указанной базе, аналогично другис SubexecTo, но вместо сохранения в каком либо виде, эти команды выводят результат в виде sql кода.
Пример
```sql
select * 
from table1
where id in (
	SubexecToArray 
	{
	-- {LOCAL_SQLITE_DB} {SQLiteDB.db} 
		QueryForLoop 
		{
		-- {str} {1,2,3,4,5}
			select str as column1
		}
	}
)
```

SubexecToUnionAllTable - Выводит результат запроса в виде повторяющихся select ... union all select ...
```
select 'd1' as 'col1','d2' as 'col2','d3' as 'col3' 
union all select 'd1, 'd2', 'd3' 
union all select 'd1, 'd2', 'd3' 
union all select 'd1, 'd2', 'd3' 
```
SubexecToArray - выводит данные в виде одного массива 'd1', 'd2', 'd3', 'd4'...

SubexecToMagic - Сделано специально для Oracle.
В Oracle есть ограничение на оператор in, value in (1,2,3,... 1000) в 1000 элементов
Однако синтаксис ('any string', value) in (('any string', 1),('any string', 2),('any string', 3),... ('any string', 10000)) позволяет достичь 10000 элементов
Функция SubexecToMagic выводит данные в виде массива ('magic', 1),('magic', 2),('magic', 3)

# Keyboard Shortcuts
```
CTRL + R / CTRL + ENTER - запустить скрипт. При нажатии либо произведётся запуск либо от ;/начала файла до ;/конца файла, либо виделенного текста
CTRL + E - Excport to excel. Аналогично кнопке на интерфейсе
CTRL + O - открыть последний сохранённый Excel  файл
CTRL + SPACE / Tab - автоподстановка 
CTRL + N - Открыть ещё один инстанс программы
CTRL + B - Закомментировать/Разкомментировать
CTRL + S - Сохранить вокрспейс

CTRL + H - Открыть историю всех запущенных скриптов
CTRL + W - Окно вокрспейсов + сохранить сейчашний.

Из таблицы/выгрузки
CTRL + C - копировать выделенное в формате excel 
  column1  column2  column3
  column1  column2  column3
CTRL + SHIFT + C - Копировать в формате sql ()
  'val1',
  'val2',
  'val3',

CTRL + I - старое окно для итерации, лучше использовать "доп_синтаксис"
```

# Misc
userdata.txt - вся информация по базам данных и пользователе
добавлять новые подключения по примеру
{Имя базы данных}.txt - создаётся при подключении к бд, хранит структуру бд. Если добавились таблицы / появился доступ - удалить файл.

user_get_tables_columns_script.sql - скрипт, за счет которого достаётся вся структура бд для автоподстановки и подсветки 
Можно редактировать,подойдёт любой скрипт, выдающий lower(имя таблицы),lower(имя столбца), для Postgre без lower
При первом запуске проги желательно пожанглировать скриптом под разные базы данных, и подключить все необходимые базы

при запуске скрипта, выполняется либо часть от ; до ; , либо выделенная область(Выделенная область ломаная, скорее всего не запашет)
sqlBackup - хранит каждый запуск sql по дате для истории

Подсветка ключевых слов (select, from, partition, dense, as и т д) адаптируется под бд (oracle, postgres и sqlite). Не доконца качественно, но на 95% корректно, ибо простого листа всех функций/ключевых слов для бд в интернете нема

qmlTestButton запускает файл DBLoadScript.qml, расположенный парралельно программе. Позволяет реализовать некоторые вспомогательные программы для разных мелочей 

Token processor test обработает все sql запросы из истории (sqlBuffer), и соберёт модель, которая будет использоваться в автоподстановке, для имитации стиля кода пользователя 

