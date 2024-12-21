using OpenTK.Graphics.OpenGL;
using OpenTK.Mathematics;
using OpenTK.Windowing.Common;
using OpenTK.Windowing.Desktop;
using OpenTK.Windowing.GraphicsLibraryFramework;
using System;

namespace lab1 {
    public class Window : GameWindow {
        private Line _line;

        public Window(GameWindowSettings gameWindowSettings, NativeWindowSettings nativeWindowSettings)
            : base(gameWindowSettings, nativeWindowSettings) {
            // Начальная линия
            _line = new Line(new Vector2(-0.5f, -0.5f), new Vector2(0.5f, 0.5f));
        }

        protected override void OnLoad() {
            base.OnLoad();
            GL.ClearColor(0.1f, 0.1f, 0.2f, 1.0f);

            _line.Load();
        }

        protected override void OnUnload() {
            _line.Unload();
            base.OnUnload();
        }

        protected override void OnRenderFrame(FrameEventArgs e) {
            base.OnRenderFrame(e);
            GL.Clear(ClearBufferMask.ColorBufferBit);

            _line.Draw();

            SwapBuffers();
        }

        protected override void OnUpdateFrame(FrameEventArgs e) {
            base.OnUpdateFrame(e);

            var input = KeyboardState;

            // Управление линией
            if (input.IsKeyDown(Keys.W)) _line.Move(new Vector2(0, 0.0001f)); // Вверх
            if (input.IsKeyDown(Keys.S)) _line.Move(new Vector2(0, -0.0001f)); // Вниз
            if (input.IsKeyDown(Keys.A)) _line.Move(new Vector2(-0.0001f, 0)); // Влево
            if (input.IsKeyDown(Keys.D)) _line.Move(new Vector2(0.0001f, 0)); // Вправо

            // Масштабирование
            if (input.IsKeyDown(Keys.Z)) _line.Scale(1.0001f);
            if (input.IsKeyDown(Keys.X)) _line.Scale(0.9999f);

            // Поворот
            if (input.IsKeyDown(Keys.Q)) _line.Rotate(0.1f);
            if (input.IsKeyDown(Keys.E)) _line.Rotate(-0.1f);

            if (input.IsKeyDown(Keys.Escape)) Close();
        }

        public void UpdateLine(Vector2 start, Vector2 end) {
            _line.UpdateVertices(start, end);
        }
    }
}
