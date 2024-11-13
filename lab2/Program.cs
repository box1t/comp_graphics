using System;
using OpenTK.Windowing.Desktop;

namespace lab2 {
    public static class Program {
        [Obsolete]
        static void Main() {
            var gameWindowSettings = GameWindowSettings.Default;
            var nativeWindowSettings = new NativeWindowSettings() {
                Size = new OpenTK.Mathematics.Vector2i(800, 600),
                Title = "3D"
            };
            
            using var window = new Window(gameWindowSettings, nativeWindowSettings);
            window.Run();
        }
    }
}