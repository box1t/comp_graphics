using OpenTK.Graphics.OpenGL;
using OpenTK.Mathematics;

namespace lab1 {

    // Базовый класс для кнопок
    public class Button {
        protected float _x { get; }
        protected float _y { get; }
        protected float _width { get; }
        protected float _height { get; }

        protected readonly float[] _coordinates;
        protected bool _isClicked = false;

        protected int _vertexBufferObject;
        protected int _vertexArrayObject;
        protected Shader _shader;

        public Button(float width, float height, float x, float y) {
            _x = x;
            _y = y;
            _width = width;
            _height = height;

            _coordinates = new float[] {
                x, y, 0.0f,
                x + width, y, 0.0f,
                x + width, y - height, 0.0f,
                x, y - height, 0.0f,
                x, y, 0.0f
            };
        }

        public virtual void Load() {
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

        public virtual void Unload() {
            GL.DeleteBuffer(_vertexBufferObject);
            GL.DeleteVertexArray(_vertexArrayObject);
            GL.DeleteProgram(_shader.Handle);
        }

        public virtual void Draw() {
            _shader.Use();

            GL.BindVertexArray(_vertexArrayObject);
            if (_isClicked)
                GL.Uniform3(_shader.GetUniformLocation("inputColor"), Color4.Gray.R, Color4.Gray.G, Color4.Gray.B);
            else
                GL.Uniform3(_shader.GetUniformLocation("inputColor"), Color4.White.R, Color4.White.G, Color4.White.B);
            GL.LineWidth(2.0f);
            GL.DrawArrays(PrimitiveType.LineLoop, 0, _coordinates.Length / 3);
        }

        public bool IsOnButton(Vector2 mousePos) {
            return mousePos.X >= _x && mousePos.X <= _x + _width &&
                   mousePos.Y <= _y && mousePos.Y >= _y - _height;
        }

        public void ClickEvent() {
            _isClicked = !_isClicked;
        }
    }

    // Класс кнопки удаления
    public class DeleteButton : Button {
        public DeleteButton(float width, float height, float x, float y) : base(width, height, x, y) { }

        public override void Draw() {
            _shader.Use();

            GL.BindVertexArray(_vertexArrayObject);
            if (!_isClicked)
                GL.Uniform3(_shader.GetUniformLocation("inputColor"), Color4.IndianRed.R, Color4.IndianRed.G, Color4.IndianRed.B);
            else
                GL.Uniform3(_shader.GetUniformLocation("inputColor"), Color4.DarkRed.R, Color4.DarkRed.G, Color4.DarkRed.B);
            GL.LineWidth(2.0f);
            GL.DrawArrays(PrimitiveType.LineLoop, 0, _coordinates.Length / 3);
        }
    }

    // Класс кнопки добавления
    public class AddButton : Button {
        public AddButton(float width, float height, float x, float y) : base(width, height, x, y) { }

        public override void Draw() {
            _shader.Use();

            GL.BindVertexArray(_vertexArrayObject);
            if (!_isClicked)
                GL.Uniform3(_shader.GetUniformLocation("inputColor"), Color4.ForestGreen.R, Color4.ForestGreen.G, Color4.ForestGreen.B);
            else
                GL.Uniform3(_shader.GetUniformLocation("inputColor"), Color4.DarkGreen.R, Color4.DarkGreen.G, Color4.DarkGreen.B);
            GL.LineWidth(2.0f);
            GL.DrawArrays(PrimitiveType.LineLoop, 0, _coordinates.Length / 3);
        }
    }

    // Класс для отрисовки границы кнопки
    public class ButtonBorder {
        private readonly float[] _coordinates;
        private int _vao;
        private int _vbo;
        private Shader _shader;

        public ButtonBorder(float width, float height, float x, float y) {
            _coordinates = new float[] {
                x, y, 0.0f,
                x + width, y, 0.0f,
                x + width, y - height, 0.0f,
                x, y - height, 0.0f,
                x, y, 0.0f
            };
        }

        public void Load() {
            _vbo = GL.GenBuffer();
            GL.BindBuffer(BufferTarget.ArrayBuffer, _vbo);
            GL.BufferData(BufferTarget.ArrayBuffer, _coordinates.Length * sizeof(float), _coordinates, BufferUsageHint.StaticDraw);

            _vao = GL.GenVertexArray();
            GL.BindVertexArray(_vao);
            GL.VertexAttribPointer(0, 3, VertexAttribPointerType.Float, false, 3 * sizeof(float), 0);
            GL.EnableVertexAttribArray(0);

            _shader = new Shader("shaders/shader.vert", "shaders/shader.frag");
            _shader.Use();
        }

        public void Draw() {
            _shader.Use();
            GL.BindVertexArray(_vao);
            GL.Uniform3(_shader.GetUniformLocation("inputColor"), Color4.Gray.R, Color4.Gray.G, Color4.Gray.B);
            GL.LineWidth(2.0f);
            GL.DrawArrays(PrimitiveType.LineLoop, 0, _coordinates.Length / 3);
        }

        public void Unload() {
            GL.DeleteVertexArray(_vao);
            GL.DeleteBuffer(_vbo);
            _shader.Dispose();
        }
    }
}