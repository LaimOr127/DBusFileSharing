# DBus File Sharing Framework

## Описание

DBus File Sharing Framework предоставляет централизованный сервис для обработки файлов с использованием сторонних приложений через DBus. Сервис позволяет регистрировать приложения, поддерживающие определенные форматы файлов, и открывать файлы с использованием зарегистрированных приложений.

## Основные возможности

- Поддержка различных форматов файлов
- Легкая конфигурация через INI-файлы
- Взаимодействие с приложениями через D-Bus
- Логирование событий через systemd journald


## Требования

- C++17 или выше
- `sdbus-c++` библиотека
- `libsystemd` библиотека
- `libinih` библиотека


## Установка зависимосстей (могу ошибаться, нужно сверятсся с оффициальной документацией)
# Для Debian/Ubuntu:
```bash
sudo apt-get install libsdbus-c++-dev libsystemd-dev libinih-dev
```
# Для Fedora:
```bash
sudo dnf install sdbus-c++-devel systemd-devel inih-devel
```


## Сборка и установка

Процесс сборки и установки прост и понятен:
```bash
$ mkdir build && cd build
$ cmake .. -DCMAKE_BUILD_TYPE=Debug
$ cmake --build . -t install
```
## Отредактируйте под ссебя файл ```sharing.ini``` в корневом каталоге проекта
```bash
# Пример файла конфигурации.
# Содержит псевдонимы приложений и их свойства.
[libreoffice-calc]
formats=ods,xlsx,xls
cmd=/usr/bin/libreoffice --calc

[libreoffice-impress]
formats=odp,pptx,ppt
cmd=/usr/bin/libreoffice --impress

[vlc]
formats=mp4,avi,mkv,mp3
cmd=/usr/bin/vlc
```

## Запуск
Чтобы запуститься с правами пользовательского пространства, необходимо ввести:
```bash
$ systemctl --user start org.rt.sharing
```
# DBusFileSharing
