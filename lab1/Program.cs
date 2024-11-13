using OpenTK.Windowing.Desktop;

namespace lab1 {
    public static class Program {
        static void Main() {
            var gameWindowSettings = GameWindowSettings.Default;
            var nativeWindowSettings = new NativeWindowSettings() {
                ClientSize = new OpenTK.Mathematics.Vector2i(800, 600),
                Title = "Polyline"
            };
            
            using var window = new Window(gameWindowSettings, nativeWindowSettings);
            window.Run();
        }
    }
}