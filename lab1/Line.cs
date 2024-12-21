using System;
using OpenTK.Graphics.OpenGL;
using OpenTK.Mathematics;

namespace lab1 {
public class Line {
    private Vector2 _start; // Начало линии
    private Vector2 _end;   // Конец линии
    private float _rotationAngle = 0.0f;
    private float _scale = 1.0f;
    private Vector2 _position = Vector2.Zero;

    private int _vertexBufferObject;
    private int _vertexArrayObject;
    private Shader _shader;

    public Line(Vector2 start, Vector2 end) {
        _start = start;
        _end = end;
    }

    public void UpdateVertices(Vector2 start, Vector2 end) {
        _start = start;
        _end = end;

        float[] vertices = {
            _start.X, _start.Y, 0.0f,
            _end.X, _end.Y, 0.0f
        };

        GL.BindBuffer(BufferTarget.ArrayBuffer, _vertexBufferObject);
        GL.BufferSubData(BufferTarget.ArrayBuffer, IntPtr.Zero, vertices.Length * sizeof(float), vertices);
    }

    public void Load() {
        float[] vertices = {
            _start.X, _start.Y, 0.0f,
            _end.X, _end.Y, 0.0f
        };

        _vertexBufferObject = GL.GenBuffer();
        GL.BindBuffer(BufferTarget.ArrayBuffer, _vertexBufferObject);
        GL.BufferData(BufferTarget.ArrayBuffer, vertices.Length * sizeof(float), vertices, BufferUsageHint.DynamicDraw);

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
        _shader.Dispose();
    }

    public void Draw() {
        _shader.Use();

        Matrix4 transform = Matrix4.Identity;
        transform *= Matrix4.CreateScale(_scale);
        transform *= Matrix4.CreateRotationZ(MathHelper.DegreesToRadians(_rotationAngle));
        transform *= Matrix4.CreateTranslation(_position.X, _position.Y, 0.0f);

        int transformLoc = _shader.GetUniformLocation("transform");
        GL.UniformMatrix4(transformLoc, false, ref transform);

        GL.BindVertexArray(_vertexArrayObject);
        GL.LineWidth(3.0f);
        GL.Uniform3(_shader.GetUniformLocation("inputColor"), 1.0f, 1.0f, 0.0f); // Жёлтый цвет
        GL.DrawArrays(PrimitiveType.Lines, 0, 2);
    }

    public void Move(Vector2 delta) {
        _position += delta;
    }

    public void Scale(float factor) {
        _scale *= factor;
    }

    public void Rotate(float angle) {
        _rotationAngle += angle;
    }
}


}
