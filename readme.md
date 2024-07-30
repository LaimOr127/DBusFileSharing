# DBus File Sharing Framework

## Описание

DBus File Sharing Framework предоставляет централизованный сервис для обработки файлов с использованием сторонних приложений через DBus. Сервис позволяет регистрировать приложения, поддерживающие определенные форматы файлов, и открывать файлы с использованием зарегистрированных приложений.

### Методы

- `void RegisterService(name: String, supportedFormats: Array<String>)`
  - Регистрирует сторонний DBus сервис с указанными форматами файлов.
  
- `void OpenFile(path: String)`
  - Открывает файл одним из зарегистрированных DBus сервисов.

- `void OpenFileUsingService(path: String, service: String)`
  - Открывает файл указанным DBus сервисом.



## Build & installation

Building & install process is straight-forward and as simple as:
```bash
$ mkdir build && cd build
$ cmake .. -DCMAKE_BUILD_TYPE=Debug
$ cmake --build . -t install
```

## Running

To run in under user-space permissions, one should type:
```bash
$ systemctl --user start org.rt.sharing
```
# DBusFileSharing
