using System;
using OpenTK.Graphics.OpenGL;
using System.Numerics;
using OpenTK.Windowing.Common;
using OpenTK.Windowing.Desktop;
using OpenTK.Windowing.GraphicsLibraryFramework;
using OpenTK.Mathematics;

namespace lab1 {
    public class Window(GameWindowSettings gameWindowSettings, NativeWindowSettings nativeWindowSettings)
        : GameWindow(gameWindowSettings, nativeWindowSettings) {
        Polyline polyline = new Polyline();

        Button _deleteButton = new DeleteButton(0.4f, 0.15f, -0.9f, 0.9f);
        Button _addButton = new AddButton(0.4f, 0.15f, -0.9f, 0.7f);

        ButtonBorder _deleteButtonBorder = new ButtonBorder(0.4f, 0.15f, -0.9f, 0.9f);
        ButtonBorder _addButtonBorder = new ButtonBorder(0.4f, 0.15f, -0.9f, 0.7f);

        protected override void OnLoad() {
            base.OnLoad();
            GL.ClearColor(Color4.Black);

            polyline.Load();
            _deleteButton.Load();
            _deleteButtonBorder.Load();
            _addButton.Load();
            _addButtonBorder.Load();

            Console.WriteLine("Window loaded and OpenGL context initialized.");
        }

        protected override void OnUnload() {
            polyline.Unload();
            _deleteButton.Unload();
            _deleteButtonBorder.Unload();
            _addButton.Unload();
            _addButtonBorder.Unload();

            base.OnUnload();
        }

        protected override void OnRenderFrame(FrameEventArgs e) {
            base.OnRenderFrame(e);
            GL.Clear(ClearBufferMask.ColorBufferBit);

            polyline.Draw();
            _deleteButton.Draw();
            _deleteButtonBorder.Draw();
            _addButton.Draw();
            _addButtonBorder.Draw();

            SwapBuffers();
        }

        protected override void OnUpdateFrame(FrameEventArgs e) {
            base.OnUpdateFrame(e);
            var input = KeyboardState;

            polyline.Animate(e.Time);

            if (input.IsKeyDown(Keys.Escape))
                Close();
            else if (input.IsKeyDown(Keys.Delete))
                polyline.DeletePoint();
            else if (input.IsKeyDown(Keys.Space) && !input.WasKeyDown(Keys.Space))
                polyline.TurnAnimation();
            else if (input.IsKeyDown(Keys.Up))
                polyline.IncreaseSpeed();
            else if (input.IsKeyDown(Keys.Down))
                polyline.ReduceSpeed();
                
            HandleMouseInput();
        }

        protected override void OnResize(ResizeEventArgs e) {
            base.OnResize(e);
            GL.Viewport(0, 0, Size.X, Size.Y);
        }


        private void HandleMouseInput() {
            var mouseState = MouseState;

            System.Numerics.Vector2 mousePos = new System.Numerics.Vector2(
                (mouseState.X / (float)Size.X) * 2f - 1f,
                -((mouseState.Y / (float)Size.Y) * 2f - 1f)
            );

            if (mouseState.IsButtonDown(MouseButton.Left) && !mouseState.WasButtonDown(MouseButton.Left)) {
                if (_deleteButton.IsOnButton(mousePos)) {
                    polyline.DeleteLastPoint();
                    _deleteButton.ClickEvent();
                }
                else if (_addButton.IsOnButton(mousePos)) {
                    if (!polyline._isInAddPointMode) polyline.EnterAddPointMode();
                    else polyline.ExitAddPointMode();
                    _addButton.ClickEvent();
                } 
                else if (polyline._isInAddPointMode) 
                    polyline.AddPoint(mousePos);
            } 
            else if (mouseState.IsButtonDown(MouseButton.Left))
                polyline.SelectPoint(mousePos);
            else if (mouseState.IsButtonDown(MouseButton.Right) && !mouseState.WasButtonDown(MouseButton.Right)) {
                polyline.DeselectPoint();
                polyline.AddPoint(mousePos);
            }
            else if (!mouseState.IsButtonDown(MouseButton.Left) && mouseState.WasButtonDown(MouseButton.Left)) {
                if (_deleteButton.IsOnButton(mousePos))
                    _deleteButton.ClickEvent();
            }
            else 
                polyline.DeselectPoint();
        }
    }
}