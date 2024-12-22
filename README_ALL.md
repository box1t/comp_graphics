# Как запустить проект


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
using OpenTK.Mathematics;
using OpenTK.Windowing.Common;
using OpenTK.Windowing.Desktop;
using OpenTK.Windowing.GraphicsLibraryFramework;
using System;
```

