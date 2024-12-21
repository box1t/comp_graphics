using OpenTK.Windowing.Desktop;

namespace lab1 {
    public static class Program {
        static void Main(string[] args) {
            // Настройка параметров окна
            var nativeWindowSettings = new NativeWindowSettings() {
                ClientSize = new OpenTK.Mathematics.Vector2i(800, 600),
                Title = "Lab-1: Отрисовка и трансформация линии"
            };
            using (var window = new Window(GameWindowSettings.Default, nativeWindowSettings)) {
                window.Run();
            }
        }
    }
}
