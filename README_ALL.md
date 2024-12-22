# Как запустить проект

## Labs 1-2 (на C#)
```sh
cd /home/snowwy/Desktop/code/_0alg/git/comp_graphics/comp_graphics/
dotnet restore
dotnet build
dotnet run
```

- Для использования шрифтов и не только:
```sh
sudo apt install libfreetype6-dev
dotnet add package SharpFont
dotnet add package SharpFont.Dependencies
dotnet add package OpenTK
```


- Часто встречаемые using`и:
```C#
using OpenTK.Graphics.OpenGL;
using OpenTK.Graphics.OpenGL4;
using OpenTK.Mathematics;
using OpenTK.Windowing.Common;
using OpenTK.Windowing.Desktop;
using OpenTK.Windowing.GraphicsLibraryFramework;
using System;
using System.Collections.Generic;
```

## Lab3+ (на C++)

1. Собираем CMakeLists.txt
2. Перейдите в папку build, очистите старую сборку, пересоздайте новую, запустите
3. Повторять CTRL-V при каждом изменении.

```sh

cd build
rm -rf *
cmake ..
cmake --build .
./l4
cd ..

```

> Prompt: 
> Помоги собрать cmakelists для данного файла и напиши инструкцию по сборке

## Lab 4

- Dependencies to install:

```sh
sudo apt update
sudo apt install libglew-dev
sudo apt install libglfw3-dev
sudo apt install libglm-dev

```

