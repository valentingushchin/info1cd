# info1cd
## Утилита командной строки для получения информации о конфигурации файловой базы 1С

Проект для Qt Creator (https://www.qt.io/). Протестировано для Qt 5.10.0 msvc2017 - 64 bit (Использовались сборочные инструменты Microsoft Build Tools 2017).<br>
В проекте используется zlib 1.2.11. Библиотеки zlib также собраны msvc2017 - 64 bit.<br>
В настройках проекта Qt Creator для используемых конфигураций (Debug и/или Release), в разделе Build Environment необходимо добавить переменную среды PROJECTDIR = <путь до директории с проектом>.

Ключи командной строки:

-j (--json) - вывод информации в формате json;<br>
-e (--encode) - выбор кодировки выводимой информации для корректного отображения наименования конфигурации.
    Варианты кодировки: UTF-8, CP-1251, CP-866(по умолчанию).

Параметр: Имя файла база данных.

![Image alt](https://github.com/valentingushchin/resource/raw/master/images/info1cd.png)

