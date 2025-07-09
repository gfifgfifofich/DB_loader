<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ru_RU" sourcelanguage="en_US">
<context>
    <name>DocWindow</name>
    <message>
        <location filename="docwindow.ui" line="14"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_docwindow.h" line="108"/>
        <source>Documentation</source>
        <translation>Документация</translation>
    </message>
    <message>
        <location filename="docwindow.ui" line="52"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_docwindow.h" line="109"/>
        <source>TextLabel</source>
        <translation></translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="83"/>
        <source>This program adds a few commands to preprocess your sql code and do a little bit of automation. Theese commands highlighter yellow by default. Highlight colors can be changed in settings</source>
        <translation>Эта программа добавляет несколько команд для предварительной обработки SQL-кода и автоматизации. Эти команды по умолчанию подсвечиваются жёлтым цветом. Цвет подсвечивания можно изменить в настройках</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="89"/>
        <source>All commands have same syntax, first goes command name, then {} and arguments each in same brackets. the rest of code inside main function brackets will be used or discarded depending on function. (e.g. ForLoop will use it as code to loop)

 Example command: </source>
        <translation>Все команды имеют одинаковый синтаксис: сначала идёт имя команды, затем {} и аргументы, каждый в тех же скобках. Остальной код внутри скобок основной функции будет использоваться или отбрасываться в зависимости от функции. (например, ForLoop будет использовать его как код для цикла)

 Пример команды: </translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="104"/>
        <source>All commands are executed from top to bottom, left to right, and can be used inside themselfs. Here is a code do do 5 queries in local database, using forloop and subexec&apos;s.</source>
        <translation>Все команды выполняются сверху вниз, слева направо и могут использоваться внутри друг друга. Вот код, который выполняет 5 запросов к локальной базе данных, используя цикл for и подкоманды.</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="122"/>
        <source>Execution commands are commands that can execute queries and do something with result</source>
        <translation>Команды выполнения — это команды, которые могут выполнять запросы и что-то делать с результатами</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="128"/>
        <location filename="docwindow.cpp" line="168"/>
        <source>Most theese commands are SubexexSomethingSomething, which are going to return status of execution back to query. folowing code shows an example of such automation with debug information used</source>
        <translation>Большинство этих команд — это SubexexSomethingSomething, которые возвращают статус выполнения обратно запросу. Следующий код показывает пример такой автоматизации с использованием отладочной информации</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="145"/>
        <source>Most of Subexecs have an analog, with &apos;Silent&apos; prefix. Theese commands are executed without returning anything to code</source>
        <translation>Большинство подкоманд имеют аналог с префиксом «Silent». Эти команды выполняются без возврата каких-либо данных в код</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="162"/>
        <source>FileImport commands are commands to automate file importing</source>
        <translation>Команды FileImport — это команды для автоматизации импорта файлов</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="185"/>
        <source>Commands to not type a lot of repetitions, due to SQL not having a basic forloops</source>
        <translation>Команды не следует вводить слишком много повторений, так как в SQL нет базовых циклов for</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="202"/>
        <source>Import excel table, paste first or selected column as oracle-style-ish array: (&apos;magic&apos;,&apos;data1&apos;),(&apos;magic&apos;,&apos;data2&apos;)</source>
        <translation>Импортируйте таблицу Excel, вставьте первый или выбранный столбец как массив в стиле Oracle: (&apos;magic&apos;,&apos;data1&apos;),(&apos;magic&apos;,&apos;data2&apos;)</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="232"/>
        <source>For loop can be used in c-style, with numbers, or to iterate over array of strings


 c-style For loop to iterate from 0 to 5 (6 iterations)</source>
        <translation>Цикл For можно использовать C-подобный, с числами или для итерации по массиву строк.

Цикл For C-подобный для итерации от 0 до 5 (6 итераций)</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="238"/>
        <location filename="docwindow.cpp" line="291"/>
        <source>Arguments:

    iterator_value - text that will be replaced in loop body. 

    from/list - may be eather a value ({1} or {10} or {023}), or a list of items, separated by comma {item1,item2,item3}.

    to - end loop when iter_value will reach this number.</source>
        <translation>Аргументы:

iterator_value — текст, который будет замещен в теле цикла.

from/list — может быть либо значением ({1}, {10} или {023}), либо списком элементов, разделённых запятой {item1,item2,item3}.

to — завершить цикл, когда iter_value достигнет этого числа.</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="256"/>
        <source>c-style For loop to iterate from 0 to 5 (6 iterations). In this case values will be pasted as 00,01,02,03,04,05... 10,11,12 ect.</source>
        <translation>Цикл For в стиле C для перебора значений от 0 до 5 (6 итераций). В этом случае значения будут вставлены как 00, 01, 02, 03, 04, 05... 10, 11, 12 и т.д.</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="269"/>
        <source>For loop ovar array of strings:</source>
        <translation>Для цикла по массиву строк:</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="284"/>
        <source>Works exactly as ForLoop, but will paste &apos;unian all&apos; between iterations. If a valid query is pasted inside this loop, result will be valid</source>
        <translation>Работает точно так же, как ForLoop, но вставляет &apos;unian all&apos; между итерациями. Если в этот цикл вставлен корректный запрос, результат будет корректным</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="313"/>
        <source>A command to execute code in a separate DatabaseConnection, and save result in a table in local database. Soon will be renamed to SubexecToLocalTable.</source>
        <translation>Команда для выполнения кода в отдельном подключении к базе данных и сохранения результата в таблице локальной базы данных. Скоро будет переименована в SubexecToLocalTable.</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="319"/>
        <location filename="docwindow.cpp" line="380"/>
        <location filename="docwindow.cpp" line="425"/>
        <location filename="docwindow.cpp" line="470"/>
        <location filename="docwindow.cpp" line="515"/>
        <location filename="docwindow.cpp" line="566"/>
        <location filename="docwindow.cpp" line="617"/>
        <location filename="docwindow.cpp" line="656"/>
        <location filename="docwindow.cpp" line="695"/>
        <source>Parameters:</source>
        <translation>Параметры:</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="325"/>
        <location filename="docwindow.cpp" line="386"/>
        <location filename="docwindow.cpp" line="431"/>
        <location filename="docwindow.cpp" line="476"/>
        <location filename="docwindow.cpp" line="521"/>
        <location filename="docwindow.cpp" line="572"/>
        <location filename="docwindow.cpp" line="623"/>
        <location filename="docwindow.cpp" line="662"/>
        <location filename="docwindow.cpp" line="701"/>
        <source>    Driver - driver wich will be used in connection</source>
        <translation>    Драйвер - драйвер, который будет использоваться в соединении</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="331"/>
        <location filename="docwindow.cpp" line="392"/>
        <location filename="docwindow.cpp" line="437"/>
        <location filename="docwindow.cpp" line="482"/>
        <location filename="docwindow.cpp" line="527"/>
        <location filename="docwindow.cpp" line="578"/>
        <location filename="docwindow.cpp" line="629"/>
        <location filename="docwindow.cpp" line="668"/>
        <location filename="docwindow.cpp" line="707"/>
        <source>    Database - Database in wich the code will be executed</source>
        <translation>    База данных - База данных, в которой будет выполняться код</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="337"/>
        <source>    Tablename - Name of local table. Local table will be replaced, if exists</source>
        <translation>    Tablename — имя локальной таблицы. Локальная таблица будет заменена, если существует</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="342"/>
        <source>This code will create a table &quot;Cool_New_Table&quot; in a local database, containing a result of this query, e.g a single column &quot;test&quot; with a single row. In this case, code will be executed in sqlite database using sqlite driver</source>
        <translation>Этот код создаст таблицу &quot;Cool_New_Table&quot; в локальной базе данных, содержащую результат этого запроса, например, один столбец &quot;test&quot; с одной строкой. В этом случае код будет выполнен в базе данных SQLite с использованием драйвера SQLite</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="355"/>
        <source>Exammple code to do preparation of data and then run main query in local database. Database/drivers can be easily replaced</source>
        <translation>Пример кода для подготовки данных и последующего выполнения основного запроса в локальной базе данных. Базу данных/драйверы можно легко заменить</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="374"/>
        <source>A command to execute code in a separate DatabaseConnection, and save result in a .csv file, located in documents/DBLoader/CSV/Filename.csv. Overwrites file if already exists. Will fail to save if file was opened in something like Excel</source>
        <translation>Команда для выполнения кода в отдельном подключении к базе данных и сохранения результата в CSV-файле, расположенном в папке documents/DBLoader/CSV/Filename.csv. Перезаписывает существующий файл. Сохранение не произойдет, если файл открыт в Excel</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="398"/>
        <location filename="docwindow.cpp" line="443"/>
        <location filename="docwindow.cpp" line="488"/>
        <location filename="docwindow.cpp" line="533"/>
        <location filename="docwindow.cpp" line="584"/>
        <source>    Filename - name of file</source>
        <translation>    Имя файла - имя файла</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="410"/>
        <source>This code will create a table &quot;Cool_New_csv.csv&quot; documents/DBLoader/CSV/Filename.csv, containing a result of this query, e.g a single column &quot;test&quot; with a single row</source>
        <translation>Этот код создаст таблицу «Cool_New_csv.csv» documents/DBLoader/CSV/Filename.csv, содержащую результат этого запроса, например, один столбец «test» с одной строкой</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="419"/>
        <source>A command to execute code in a separate DatabaseConnection, and save result in a .csv file, located in documents/DBLoader/CSV/Filename.csv. Appends data to file. Will fail to save if file was opened in something like Excel</source>
        <translation>Команда для выполнения кода в отдельном подключении к базе данных и сохранения результата в CSV-файле, расположенном в папке documents/DBLoader/CSV/Filename.csv. Добавляет данные в файл. Сохранение данных не произойдет, если файл открыт в Excel</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="455"/>
        <source>This code will create a table &quot;Cool_New_csv.csv&quot; documents/DBLoader/CSV/Filename.csv, containing a result of this query, e.g a single column &quot;test&quot; with a single row. If executed multiple times, will add more rows to that file</source>
        <translation>Этот код создаст таблицу &quot;Cool_New_csv.csv&quot; в файле documents/DBLoader/CSV/Filename.csv, содержащую результат запроса, например, один столбец &quot;test&quot; с одной строкой. При многократном выполнении в этот файл будут добавлены новые строки</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="464"/>
        <location filename="docwindow.cpp" line="509"/>
        <location filename="docwindow.cpp" line="560"/>
        <source>A command to execute code in a separate DatabaseConnection, and save result in a .xlsx file, located in documents/DBLoader/excel/filename.xlsx. Overwrites file if already exists. Will fail to save if file was opened in something like Excel</source>
        <translation>Команда для выполнения кода в отдельном подключении к базе данных и сохранения результата в файле .xlsx, расположенном в папке documents/DBLoader/excel/имя_файла.xlsx. Перезаписывает существующий файл. Сохранение не произойдет, если файл открыт в Excel</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="500"/>
        <source>This code will create a table &quot;Cool_New_xlsx.xlsx&quot; in documents/DBLoader/excel/filename.xlsx, containing a result of this query, e.g a single column &quot;test&quot; with a single row</source>
        <translation>Этот код создаст таблицу «Cool_New_xlsx.xlsx» в documents/DBLoader/excel/filename.xlsx, содержащую результат этого запроса, например, один столбец «test» с одной строкой</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="539"/>
        <location filename="docwindow.cpp" line="590"/>
        <source>    SheetName - name of Sheet in excel table</source>
        <translation>    SheetName - имя листа в таблице Excel</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="551"/>
        <source>This code will create a table &quot;Cool_New_xlsx.xlsx&quot; in documents/DBLoader/excel/filename.xlsx, containing a result of this query, e.g a single column &quot;test&quot; with a single row. Data will be located on a sheet &quot;SheetName&quot;, overwriting all data on that list. If file contained pivottables or other info, it will be deleated, use SubexecAppendExcelWorksheet{}.</source>
        <translation>Этот код создаст таблицу &quot;Cool_New_xlsx.xlsx&quot; в каталоге documents/DBLoader/excel/filename.xlsx, содержащую результат этого запроса, например, один столбец &quot;test&quot; с одной строкой. Данные будут размещены на листе &quot;SheetName&quot;, перезаписывая все данные в этом списке. Если файл содержал сводные таблицы или другую информацию, он будет удалён с помощью функции SubexecAppendExcelWorksheet{}.</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="602"/>
        <source>This code will create a table &quot;Cool_New_xlsx.xlsx&quot; in documents/DBLoader/excel/filename.xlsx, containing a result of this query, e.g a single column &quot;test&quot; with a single row. Data will be located on a sheet &quot;SheetName&quot;, overwriting all data on that list. new Data will be placed below other data on the list. Somewhat scuffed due to use of ODBC_Excel driver, but data is safe</source>
        <translation>Этот код создаст таблицу &quot;Cool_New_xlsx.xlsx&quot; в каталоге documents/DBLoader/excel/filename.xlsx, содержащую результат этого запроса, например, один столбец &quot;test&quot; с одной строкой. Данные будут размещены на листе &quot;SheetName&quot;, перезаписывая все данные в этом списке. Новые данные будут размещены под другими данными в списке. Данные немного повреждены из-за использования драйвера ODBC_Excel, но в безопасности</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="611"/>
        <source>A command to execute code in a separate DatabaseConnection, and paste result in a oracle-specific format of arrays to allow more rows beeng put into &apos;in()&apos; statement format: (&apos;magic&apos;,&apos;data1&apos;),(&apos;magic&apos;,&apos;data2&apos;)</source>
        <translation>Команда для выполнения кода в отдельном DatabaseConnection и вставки результата в специфичный для Oracle формат массивов, что позволяет поместить больше строк в формат оператора &apos;in()&apos;: (&apos;magic&apos;,&apos;data1&apos;),(&apos;magic&apos;,&apos;data2&apos;)</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="641"/>
        <location filename="docwindow.cpp" line="680"/>
        <source>This is a crude way to awoid dblinks, or get data depending on external sources.</source>
        <translation>Это грубый способ избежать бликов или получения данных в зависимости от внешних источников.</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="650"/>
        <source>A command to execute code in a separate DatabaseConnection, and paste result in standart-ish sql array format: &apos;data1&apos;, &apos;data2&apos;, &apos;data3&apos; </source>
        <translation>Команда для выполнения кода в отдельном DatabaseConnection и вставки результата в стандартный формат массива SQL: &apos;data1&apos;, &apos;data2&apos;, &apos;data3&apos; </translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="689"/>
        <source>A command to execute code in a separate DatabaseConnection, and paste result in a bunch of select &apos;a&apos; as &quot;columnname&quot; union all select &apos;b&apos; as &quot;columnname&quot; </source>
        <translation>Команда для выполнения кода в отдельном DatabaseConnection и вставки результата в группу select &apos;a&apos; as &quot;columnname&quot; union all select &apos;b&apos; as &quot;columnname&quot; </translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="719"/>
        <source>most Databases are wery slow with this type of actions</source>
        <translation>Большинство баз данных очень медленно справляются с такими действиями</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="728"/>
        <source>Do not use without AwaitAsyncExecution{}</source>
        <translation>Не использовать без AwaitAsyncExecution{}</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="734"/>
        <source>All subexecs after this command and &apos;AwaitAsyncExecution{}&apos; will be launched in parallel</source>
        <translation>Все подпроцессы после этой команды и &apos;AwaitAsyncExecution{}&apos; будут запущены параллельно</translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="742"/>
        <source>This example will launch 5 subexecs in local database in parallel: </source>
        <translation>В этом примере параллельно запустятся 5 подпроцессов в локальной базе данных: </translation>
    </message>
    <message>
        <location filename="docwindow.cpp" line="758"/>
        <source>A command to wait for all async subexecs to finish, and process what their commands (save to table/ paste stuff, ect.)</source>
        <translation>Команда, которая ждет завершения всех асинхронных подпрограмм и обрабатывает их команды (сохранение в таблицу, вставка данных и т. д.)</translation>
    </message>
</context>
<context>
    <name>LoaderWidnow</name>
    <message>
        <location filename="loaderwidnow.ui" line="20"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="646"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="674"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="646"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="724"/>
        <source>Loader</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="54"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="713"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="742"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="713"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="787"/>
        <source>Driver</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="76"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="714"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="743"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="714"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="788"/>
        <source>Database name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="89"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="715"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="744"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="715"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="789"/>
        <source>User name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="105"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="716"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="745"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="716"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="790"/>
        <source>Password</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="118"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="717"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="746"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="717"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="791"/>
        <source>Connect</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="133"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="718"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="747"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="718"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="792"/>
        <source>Connection status:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="146"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="719"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="748"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="719"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="793"/>
        <source>Not connected</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="206"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="721"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="750"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="721"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="795"/>
        <source>pos</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="228"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="722"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="751"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="722"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="796"/>
        <source>status</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="235"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="723"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="752"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="723"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="797"/>
        <source>load 500 lines only</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="242"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="724"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="753"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="724"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="798"/>
        <source>Run query</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="249"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="725"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="754"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="725"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="799"/>
        <source>Send stop signal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="280"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="729"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="755"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="729"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="803"/>
        <source>Tab 2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="294"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="730"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="757"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="730"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="804"/>
        <source>Add tab</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="318"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="731"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="758"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="731"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="805"/>
        <source>Script autolauncher</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="331"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="732"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="759"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="732"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="806"/>
        <source>Next launch at:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="344"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="733"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="760"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="733"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="807"/>
        <source>Last launch was </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="353"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="734"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="761"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="734"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="808"/>
        <source>Each Day at</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="378"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="735"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="762"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="735"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="809"/>
        <source>Day Of Month</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="413"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="736"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="763"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="736"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="810"/>
        <source>Day Of Week</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="455"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="769"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="764"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="769"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="843"/>
        <source>File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="463"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="770"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="765"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="770"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="844"/>
        <source>Settings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="469"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="772"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="766"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="772"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="846"/>
        <source>Help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="475"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="773"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="767"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="773"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="847"/>
        <source>Misc</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="484"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="774"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="768"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="774"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="848"/>
        <source>Code</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="493"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="775"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="769"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="775"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="849"/>
        <source>Windows</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="503"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="776"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="770"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="776"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="850"/>
        <source>Data</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="527"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="771"/>
        <source>Table data</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="552"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="737"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="772"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="737"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="811"/>
        <source>tmp</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="555"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="738"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="773"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="738"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="812"/>
        <source>Save name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="574"/>
        <location filename="loaderwidnow.ui" line="863"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="701"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="742"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="729"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="777"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="701"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="742"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="779"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="816"/>
        <source>Save as xlsx</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="590"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="746"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="781"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="746"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="820"/>
        <source>Save as CSV</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="615"/>
        <location filename="loaderwidnow.ui" line="876"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="706"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="756"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="734"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="791"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="706"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="756"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="784"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="830"/>
        <source>Save as SQLite table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="622"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="757"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="792"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="757"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="831"/>
        <source>Import from CSV file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="629"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="758"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="793"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="758"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="832"/>
        <source>Import from Excel file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="646"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="759"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="794"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="759"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="833"/>
        <source>Data size:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="659"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="760"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="795"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="760"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="834"/>
        <source>No data</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="672"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="761"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="796"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="761"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="835"/>
        <source>No database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="679"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="762"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="797"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="762"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="836"/>
        <source>nnTestLearn</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="686"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="763"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="798"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="763"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="837"/>
        <source>nnTestRun</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="693"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="764"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="799"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="764"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="838"/>
        <source>Token processor test</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="700"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="765"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="800"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="765"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="839"/>
        <source>qml test button</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="725"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="647"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="675"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="647"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="725"/>
        <source>UserTheme</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="730"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="648"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="676"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="648"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="726"/>
        <source>Key bindings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="735"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="677"/>
        <source>Documentation</source>
        <translation type="unfinished">Документация</translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="740"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="650"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="678"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="650"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="728"/>
        <source>no.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="745"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="651"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="679"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="651"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="729"/>
        <source>Run token processor</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="750"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="652"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="680"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="652"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="730"/>
        <source>NN Learn first column</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="755"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="653"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="681"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="653"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="731"/>
        <source>NN Run</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="760"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="654"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="682"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="654"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="732"/>
        <source>Open QML form</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="765"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="655"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="683"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="655"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="733"/>
        <source>Run</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="768"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="657"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="685"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="657"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="735"/>
        <source>Ctrl+Return</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="773"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="659"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="687"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="659"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="737"/>
        <source>New window</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="776"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="661"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="689"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="661"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="739"/>
        <source>Ctrl+Alt+N</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="781"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="663"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="691"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="663"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="741"/>
        <source>Graph</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="784"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="665"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="693"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="665"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="743"/>
        <source>Ctrl+G</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="789"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="667"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="695"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="667"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="745"/>
        <source>History</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="792"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="669"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="697"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="669"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="747"/>
        <source>Ctrl+H</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="797"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="671"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="699"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="671"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="749"/>
        <source>Workspaces</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="800"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="673"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="701"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="673"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="751"/>
        <source>Ctrl+W</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="805"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="675"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="703"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="675"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="753"/>
        <source>Autorun</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="808"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="677"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="705"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="677"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="755"/>
        <source>Ctrl+T</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="813"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="679"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="707"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="679"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="757"/>
        <source>CommentSelected</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="816"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="681"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="709"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="681"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="759"/>
        <source>Ctrl+B</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="821"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="683"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="711"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="683"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="761"/>
        <source>Move right</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="826"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="684"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="712"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="684"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="762"/>
        <source>Move left</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="831"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="685"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="713"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="685"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="763"/>
        <source>Open export directory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="834"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="687"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="715"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="687"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="765"/>
        <source>Ctrl+Alt+O</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="839"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="689"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="717"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="689"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="767"/>
        <source>Open last excel export</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="842"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="691"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="719"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="691"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="769"/>
        <source>Ctrl+O</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="847"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="693"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="721"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="693"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="771"/>
        <source>Replace</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="850"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="695"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="723"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="695"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="773"/>
        <source>Ctrl+F</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="855"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="697"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="725"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="697"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="775"/>
        <source>Save workspace</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="858"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="699"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="727"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="699"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="777"/>
        <source>Ctrl+S</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="866"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="703"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="731"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="703"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="781"/>
        <source>Ctrl+E</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="871"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="705"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="733"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="705"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="783"/>
        <source>Save as csv</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="881"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="707"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="735"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="707"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="785"/>
        <source>Import from xlsx file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="886"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="708"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="736"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="708"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="786"/>
        <source>Import from csv file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="891"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="709"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="737"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="709"/>
        <source>Describer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="894"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="711"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="739"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="711"/>
        <source>Ctrl+D</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.ui" line="899"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_loaderwidnow.h" line="741"/>
        <source>Copy last launched sql</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="649"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="649"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="727"/>
        <source>subfunction docs</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="720"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="720"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="794"/>
        <source>suggestion</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="726"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="726"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="800"/>
        <source>Workspace</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="727"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="727"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="801"/>
        <source>Workspace name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="build/610-Debug/untitled_autogen/include/ui_loaderwidnow.h" line="771"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_loaderwidnow.h" line="771"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_loaderwidnow.h" line="845"/>
        <source>Info</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.cpp" line="1958"/>
        <source>Select csv file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.cpp" line="2385"/>
        <source>Change Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="loaderwidnow.cpp" line="2386"/>
        <source>Insert New Tab Name</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="QXlsx-master/chartsquestions/mainwindow.ui" line="14"/>
        <source>MainWindow</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="QXlsx-master/chartsquestions/mainwindow.ui" line="35"/>
        <source>Chromatogram</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="QXlsx-master/chartsquestions/mainwindow.ui" line="50"/>
        <source>*.txt</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="QXlsx-master/chartsquestions/mainwindow.ui" line="62"/>
        <source>Report file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="QXlsx-master/chartsquestions/mainwindow.ui" line="77"/>
        <source>*.xlsx</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="QXlsx-master/chartsquestions/mainwindow.ui" line="89"/>
        <source>Sheet_name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="QXlsx-master/chartsquestions/mainwindow.ui" line="116"/>
        <source>precision</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="QXlsx-master/chartsquestions/mainwindow.ui" line="131"/>
        <source>0.1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="QXlsx-master/chartsquestions/mainwindow.ui" line="143"/>
        <source>Generate</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ReplaceWindow</name>
    <message>
        <location filename="replacewindow.ui" line="14"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_replacewindow.h" line="204"/>
        <source>MainWindow</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="replacewindow.ui" line="36"/>
        <location filename="replacewindow.ui" line="186"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_replacewindow.h" line="205"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_replacewindow.h" line="210"/>
        <source>Replace</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="replacewindow.ui" line="61"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_replacewindow.h" line="206"/>
        <source>With</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="replacewindow.ui" line="109"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_replacewindow.h" line="207"/>
        <source>from line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="replacewindow.ui" line="137"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_replacewindow.h" line="208"/>
        <source>to line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="replacewindow.ui" line="179"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_replacewindow.h" line="209"/>
        <source>Find next</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="replacewindow.ui" line="193"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_replacewindow.h" line="211"/>
        <source>replace  with next line</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>SettingsWindow</name>
    <message>
        <location filename="settingswindow.ui" line="501"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_settingswindow.h" line="140"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_settingswindow.h" line="140"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_settingswindow.h" line="140"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_settingswindow.h" line="140"/>
        <source>SettingsWindow</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>StructureDescriptor</name>
    <message>
        <location filename="structuredescriptor.ui" line="14"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_structuredescriptor.h" line="146"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_structuredescriptor.h" line="146"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_structuredescriptor.h" line="146"/>
        <source>Database descriptor</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="structuredescriptor.ui" line="37"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_structuredescriptor.h" line="147"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_structuredescriptor.h" line="147"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_structuredescriptor.h" line="147"/>
        <source>Tables</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="structuredescriptor.ui" line="44"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_structuredescriptor.h" line="148"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_structuredescriptor.h" line="148"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_structuredescriptor.h" line="148"/>
        <source>Columns in tables</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="structuredescriptor.ui" line="82"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_structuredescriptor.h" line="149"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_structuredescriptor.h" line="149"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_structuredescriptor.h" line="149"/>
        <source>Table columns</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="structuredescriptor.ui" line="99"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_structuredescriptor.h" line="150"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_structuredescriptor.h" line="150"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_structuredescriptor.h" line="150"/>
        <source>Update</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Table</name>
    <message>
        <location filename="table.ui" line="501"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_table.h" line="330"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_table.h" line="330"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_table.h" line="330"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_table.h" line="330"/>
        <location filename="Новая папка/untitled_autogen/include/ui_table.h" line="330"/>
        <source>MainWindow</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="table.ui" line="533"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_table.h" line="331"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_table.h" line="331"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_table.h" line="331"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_table.h" line="331"/>
        <location filename="Новая папка/untitled_autogen/include/ui_table.h" line="331"/>
        <source>Select Driver</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="table.ui" line="543"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_table.h" line="332"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_table.h" line="332"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_table.h" line="332"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_table.h" line="332"/>
        <location filename="Новая папка/untitled_autogen/include/ui_table.h" line="332"/>
        <source>Select database</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="table.ui" line="556"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_table.h" line="334"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_table.h" line="334"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_table.h" line="334"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_table.h" line="334"/>
        <location filename="Новая папка/untitled_autogen/include/ui_table.h" line="334"/>
        <source>UserName</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="table.ui" line="569"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_table.h" line="336"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_table.h" line="336"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_table.h" line="336"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_table.h" line="336"/>
        <location filename="Новая папка/untitled_autogen/include/ui_table.h" line="336"/>
        <source>Password</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="table.ui" line="576"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_table.h" line="337"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_table.h" line="337"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_table.h" line="337"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_table.h" line="337"/>
        <location filename="Новая папка/untitled_autogen/include/ui_table.h" line="337"/>
        <source>connect</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="table.ui" line="590"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_table.h" line="338"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_table.h" line="338"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_table.h" line="338"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_table.h" line="338"/>
        <location filename="Новая папка/untitled_autogen/include/ui_table.h" line="338"/>
        <source>excel/tmp.xlsx</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="table.ui" line="602"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_table.h" line="339"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_table.h" line="339"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_table.h" line="339"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_table.h" line="339"/>
        <location filename="Новая папка/untitled_autogen/include/ui_table.h" line="339"/>
        <source>Save as SQLite table</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="table.ui" line="609"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_table.h" line="340"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_table.h" line="340"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_table.h" line="340"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_table.h" line="340"/>
        <location filename="Новая папка/untitled_autogen/include/ui_table.h" line="340"/>
        <source>Save as xlsx file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="table.ui" line="625"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_table.h" line="341"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_table.h" line="341"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_table.h" line="341"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_table.h" line="341"/>
        <location filename="Новая папка/untitled_autogen/include/ui_table.h" line="341"/>
        <source>exec</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="table.ui" line="632"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_table.h" line="342"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_table.h" line="342"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_table.h" line="342"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_table.h" line="342"/>
        <location filename="Новая папка/untitled_autogen/include/ui_table.h" line="342"/>
        <source>Ignore dependencies</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="table.ui" line="642"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_table.h" line="343"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_table.h" line="343"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_table.h" line="343"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_table.h" line="343"/>
        <location filename="Новая папка/untitled_autogen/include/ui_table.h" line="343"/>
        <source>Run as script</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="table.ui" line="653"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_table.h" line="344"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_table.h" line="344"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_table.h" line="344"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_table.h" line="344"/>
        <location filename="Новая папка/untitled_autogen/include/ui_table.h" line="344"/>
        <source>WorkSpace0</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="table.ui" line="660"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_table.h" line="345"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_table.h" line="345"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_table.h" line="345"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_table.h" line="345"/>
        <location filename="Новая папка/untitled_autogen/include/ui_table.h" line="345"/>
        <source>Save</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="table.ui" line="674"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_table.h" line="346"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_table.h" line="346"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_table.h" line="346"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_table.h" line="346"/>
        <location filename="Новая папка/untitled_autogen/include/ui_table.h" line="346"/>
        <source>Suggestion: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="table.ui" line="681"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_table.h" line="347"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_table.h" line="347"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_table.h" line="347"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_table.h" line="347"/>
        <location filename="Новая папка/untitled_autogen/include/ui_table.h" line="347"/>
        <source>status</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="table.ui" line="721"/>
        <location filename="table.ui" line="726"/>
        <location filename="table.ui" line="731"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_table.h" line="352"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_table.h" line="354"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_table.h" line="356"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_table.h" line="352"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_table.h" line="354"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_table.h" line="356"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_table.h" line="352"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_table.h" line="354"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_table.h" line="356"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_table.h" line="352"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_table.h" line="354"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_table.h" line="356"/>
        <location filename="Новая папка/untitled_autogen/include/ui_table.h" line="352"/>
        <location filename="Новая папка/untitled_autogen/include/ui_table.h" line="354"/>
        <location filename="Новая папка/untitled_autogen/include/ui_table.h" line="356"/>
        <source>New Item</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>main</name>
    <message>
        <location filename="QXlsx-master/HelloAndroid/main.qml" line="11"/>
        <source>Hello Android</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>replaceDialog</name>
    <message>
        <location filename="replacedialog.ui" line="501"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_replacedialog.h" line="279"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_replacedialog.h" line="277"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_replacedialog.h" line="279"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_replacedialog.h" line="279"/>
        <source>Replace tool</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="replacedialog.ui" line="522"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_replacedialog.h" line="280"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_replacedialog.h" line="278"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_replacedialog.h" line="280"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_replacedialog.h" line="280"/>
        <source>Replace</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="replacedialog.ui" line="547"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_replacedialog.h" line="281"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_replacedialog.h" line="279"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_replacedialog.h" line="281"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_replacedialog.h" line="281"/>
        <source>With</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="replacedialog.ui" line="595"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_replacedialog.h" line="282"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_replacedialog.h" line="280"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_replacedialog.h" line="282"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_replacedialog.h" line="282"/>
        <source>from line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="replacedialog.ui" line="623"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_replacedialog.h" line="283"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_replacedialog.h" line="281"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_replacedialog.h" line="283"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_replacedialog.h" line="283"/>
        <source>to line</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="replacedialog.ui" line="681"/>
        <location filename="build/610-Debug/untitled_autogen/include/ui_replacedialog.h" line="284"/>
        <location filename="build/610-Release-new/untitled_autogen/include/ui_replacedialog.h" line="282"/>
        <location filename="build/610-RelWithDebInfo/untitled_autogen/include/ui_replacedialog.h" line="284"/>
        <location filename="build/Desktop-Release-Dev/untitled_autogen/include/ui_replacedialog.h" line="284"/>
        <source>replace  with next line</source>
        <translation type="unfinished"></translation>
    </message>
</context>
</TS>
