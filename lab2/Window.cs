using System.Threading;
using OpenTK.Graphics.OpenGL;
using OpenTK.Windowing.Common;
using OpenTK.Windowing.Desktop;
using OpenTK.Mathematics;
using OpenTK.Windowing.GraphicsLibraryFramework;

namespace lab2 {
    public class Window(GameWindowSettings gameWindowSettings, NativeWindowSettings nativeWindowSettings)
        : GameWindow(gameWindowSettings, nativeWindowSettings) {
        
        private readonly Cube cube = new();
        private readonly Cylinder cylinder = new();

        private readonly Pyramid pyramid = new();
        private const double TargetFrameTime = 1.0 / 60.0;

        private readonly VanishingPoint firstPoint = new([-0.7f, 0.5f, 0.0f]);
        private readonly VanishingPoint secondPoint = new([0.7f, 0.5f, 0.0f]);

        protected override void OnLoad() {
            base.OnLoad();
            GL.ClearColor(Color4.Black);
            GL.Enable(EnableCap.DepthTest);

            cube.Load(Size.X, Size.Y);
            // cube.Move(new System.Numerics.Vector3(-1f, 0f, 0f));

            cylinder.Load(Size.X, Size.Y);
            // cylinder.Move(new System.Numerics.Vector3(0f, 0f, 0f));

            pyramid.Load(Size.X, Size.Y);
            // pyramid.Move(new System.Numerics.Vector3(2f, 0f, 0f));

            firstPoint.Load(Size.X, Size.Y);
            secondPoint.Load(Size.X, Size.Y);
        }

        protected override void OnUnload() {
            
            cube.Unload();
            cylinder.Unload();
            pyramid.Unload();
            firstPoint.Unload();
            secondPoint.Unload();

            base.OnUnload();
        }

        protected override void OnResize(ResizeEventArgs e) {
            base.OnResize(e);
            GL.Viewport(0, 0, Size.X, Size.Y);
            cube.Update(Size.X, Size.Y);
            cylinder.Update(Size.X, Size.Y);
            pyramid.Update(Size.X, Size.Y);
        }

        protected override void OnRenderFrame(FrameEventArgs e) {
            base.OnRenderFrame(e);
            GL.Clear(ClearBufferMask.ColorBufferBit | ClearBufferMask.DepthBufferBit);

            firstPoint.Draw();
            secondPoint.Draw();

            cube.ChangePerspective(firstPoint.GetCoordinates(), secondPoint.GetCoordinates());
            cube.Draw();

            cylinder.ChangePerspective(firstPoint.GetCoordinates(), secondPoint.GetCoordinates());
            cylinder.Draw();

            pyramid.ChangePerspective(firstPoint.GetCoordinates(), secondPoint.GetCoordinates());
            pyramid.Draw();

            SwapBuffers();

            double frameTime = e.Time;
            if (frameTime < TargetFrameTime) {
                Thread.Sleep((int)((TargetFrameTime - frameTime) * 1000.0));  // Задержка для достижения целевого FPS
            }
        }

        protected override void OnUpdateFrame(FrameEventArgs e) {
            base.OnUpdateFrame(e);
            HandleKeyboardInput();
            HandleMouseInput();
        }

        private void HandleKeyboardInput() {
            var input = KeyboardState;
            if (input.IsKeyDown(Keys.Up)) {
                cube.UpView();
                cylinder.UpView();
                pyramid.UpView();
            }
            else if (input.IsKeyDown(Keys.Down)) {
                cube.DownView();
                cylinder.DownView();
                pyramid.DownView();
            }
        }
        
        private void HandleMouseInput() {
            var mouseState = MouseState;

            System.Numerics.Vector2 mousePos = new(
                (2 * mouseState.X / Size.X) - 1f,
                -((2 * mouseState.Y / Size.Y) - 0.85f)
            );

            if (mouseState.IsButtonDown(MouseButton.Left)) {
                if (!secondPoint.isSelected && !cube.isSelected && !cylinder.isSelected && !pyramid.isSelected)
                    firstPoint.SelectPoint(mousePos);
                if (!firstPoint.isSelected && !cube.isSelected && !cylinder.isSelected && !pyramid.isSelected)
                    secondPoint.SelectPoint(mousePos);
                if (!firstPoint.isSelected && !secondPoint.isSelected && !cylinder.isSelected && !pyramid.isSelected) {
                    cube.SelectPoint(mousePos);
                }
                if (!firstPoint.isSelected && !secondPoint.isSelected && !cylinder.isSelected && !cube.isSelected) {
                    pyramid.SelectPoint(mousePos);
                }
                if (!firstPoint.isSelected && !secondPoint.isSelected && !cube.isSelected && !pyramid.isSelected) {
                    cylinder.SelectPoint(mousePos);
                }
            } else {
                firstPoint.DeselectPoint();
                secondPoint.DeselectPoint();
                cube.DeselectPoint();
                pyramid.DeselectPoint();
                cylinder.DeselectPoint();
            }
        }
    }
}