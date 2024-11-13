using OpenTK.Graphics.OpenGL;
using System.Numerics;
using OpenTK.Mathematics;

namespace lab1 {
    public class DeleteButton(float width, float height, float x, float y) : Button(width, height, x, y) {
        public override void Draw() {
            _shader.Use();

            GL.BindVertexArray(_vertexArrayObject);
            if (!_isClicked)
                GL.Uniform3(_shader.GetUniformLocation("inputColor"), Color4.IndianRed.R, Color4.IndianRed.G, Color4.IndianRed.B);
            else
                GL.Uniform3(_shader.GetUniformLocation("inputColor"), Color4.IndianRed.R - 0.1f, Color4.IndianRed.G - 0.1f, Color4.IndianRed.B - 0.1f);
            GL.LineWidth(2.0f);
            GL.DrawArrays(PrimitiveType.Triangles, 0, _coordinates.Length / 3);
        }
    }

    public class AddButton(float width, float height, float x, float y) : Button(width, height, x, y) {
        public override void Draw() {
            _shader.Use();

            GL.BindVertexArray(_vertexArrayObject);
            if (!_isClicked)
                GL.Uniform3(_shader.GetUniformLocation("inputColor"), Color4.ForestGreen.R, Color4.ForestGreen.G, Color4.ForestGreen.B);
            else
                GL.Uniform3(_shader.GetUniformLocation("inputColor"), Color4.ForestGreen.R - 0.1f, Color4.ForestGreen.G - 0.1f, Color4.ForestGreen.B - 0.1f);
            GL.LineWidth(2.0f);
            GL.DrawArrays(PrimitiveType.Triangles, 0, _coordinates.Length / 3);
        }
    }

    public class ButtonBorder(float width, float height, float x, float y) {
        protected float _x { get; } = x;
        protected float _y { get; } = y;
        protected float _width { get; } = width;
        protected float _height { get; } = height;
        protected readonly float[] _coordinates = [
            x, y, 0.0f,
            x + width, y, 0.0f,
            x + width, y - height, 0.0f,
            x, y - height, 0.0f,
            x, y, 0.0f
        ];

        protected int _vertexBufferObject;
        protected int _vertexArrayObject;
        protected Shader _shader;

        public void Load() {
            _vertexBufferObject = GL.GenBuffer();
            GL.BindBuffer(BufferTarget.ArrayBuffer, _vertexBufferObject);
            GL.BufferData(BufferTarget.ArrayBuffer, _coordinates.Length * sizeof(float), _coordinates, BufferUsageHint.StaticDraw);

            _vertexArrayObject = GL.GenVertexArray();
            GL.BindVertexArray(_vertexArrayObject);
            GL.VertexAttribPointer(0, 3, VertexAttribPointerType.Float, false, 3 * sizeof(float), 0);
            GL.EnableVertexAttribArray(0);

            _shader = new Shader("shaders/shader.vert", "shaders/shader.frag");
            _shader.Use();
        }

        public void Unload() {
            GL.DeleteBuffer(_vertexBufferObject);
            GL.DeleteVertexArray(_vertexArrayObject);
            GL.DeleteProgram(_shader.Handle);
        }

        public virtual void Draw() {
            _shader.Use();

            GL.BindVertexArray(_vertexArrayObject);
            GL.Uniform3(_shader.GetUniformLocation("inputColor"), Color4.White.R, Color4.White.G, Color4.White.B);
            GL.LineWidth(2.0f);
            GL.DrawArrays(PrimitiveType.LineStrip, 0, _coordinates.Length / 3);
        }
        public bool IsOnButton(System.Numerics.Vector2 mousePos) {
            if (mousePos.X >= _x && mousePos.X <= _x + _width &&
                mousePos.Y <= _y && mousePos.Y >= _y - _height)
                return true;
            return false;
        }
    }

    public class Button(float width, float height, float x, float y) {
        protected float _x { get; } = x;
        protected float _y { get; } = y;
        protected float _width { get; } = width;
        protected float _height { get; } = height;
        protected readonly float[] _coordinates = [
            x, y, 0.0f,
            x + width, y, 0.0f,
            x + width, y - height, 0.0f,

            x + width, y - height, 0.0f,
            x, y - height, 0.0f,
            x, y, 0.0f
        ];

        private readonly float[] _border = [
            x, y, 0.0f,
            x + width, y, 0.0f,
            x, y - height, 0.0f,
            x + width, y - height, 0.0f,
            x, y, 0.0f
        ];

        protected bool _isClicked = false;

        protected int _vertexBufferObject;
        protected int _vertexArrayObject;
        protected Shader _shader;

        public void Load() {
            _vertexBufferObject = GL.GenBuffer();
            GL.BindBuffer(BufferTarget.ArrayBuffer, _vertexBufferObject);
            GL.BufferData(BufferTarget.ArrayBuffer, _coordinates.Length * sizeof(float), _coordinates, BufferUsageHint.StaticDraw);

            _vertexArrayObject = GL.GenVertexArray();
            GL.BindVertexArray(_vertexArrayObject);
            GL.VertexAttribPointer(0, 3, VertexAttribPointerType.Float, false, 3 * sizeof(float), 0);
            GL.EnableVertexAttribArray(0);

            _shader = new Shader("shaders/shader.vert", "shaders/shader.frag");
            _shader.Use();
        }

        public void Unload() {
            GL.DeleteBuffer(_vertexBufferObject);
            GL.DeleteVertexArray(_vertexArrayObject);
            GL.DeleteProgram(_shader.Handle);
        }

        public virtual void Draw() {
            _shader.Use();

            GL.BindVertexArray(_vertexArrayObject);
            GL.Uniform3(_shader.GetUniformLocation("inputColor"), Color4.White.R, Color4.White.G, Color4.White.B);
            GL.LineWidth(2.0f);
            GL.DrawArrays(PrimitiveType.Triangles, 0, _coordinates.Length / 3);
        }

        public bool IsOnButton(System.Numerics.Vector2 mousePos) {
            if (mousePos.X >= _x && mousePos.X <= _x + _width &&
                mousePos.Y <= _y && mousePos.Y >= _y - _height)
                return true;
            return false;
        }

        public void ClickEvent() {
            _isClicked = !_isClicked;
        }
    }
}