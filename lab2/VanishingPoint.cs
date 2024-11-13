using OpenTK.Graphics.OpenGL;
using OpenTK.Mathematics;

namespace lab2 {
    public class VanishingPoint(float[] _coordinates) {
        private readonly float[] coordinates = _coordinates;
        public bool isSelected = false;
        int VBO;
        private int VAO;
        private Shader? shader;

        public void Load(int width, int height) {
            VBO = GL.GenBuffer();
            GL.BindBuffer(BufferTarget.ArrayBuffer, VBO);
            GL.BufferData(BufferTarget.ArrayBuffer, coordinates.Length * sizeof(float), coordinates, BufferUsageHint.StaticDraw);

            VAO = GL.GenVertexArray();
            GL.BindVertexArray(VAO);
            GL.VertexAttribPointer(0, 3, VertexAttribPointerType.Float, false, 3 * sizeof(float), 0);
            GL.EnableVertexAttribArray(0);
            shader = new Shader("shaders/shaderPoint.vert", "shaders/shader.frag");
            shader.Use();
        }

        public void Unload() {
            GL.DeleteBuffer(VBO);
            GL.DeleteVertexArray(VAO);
            if (shader != null)
                GL.DeleteProgram(shader.Handle);
        }

        public void Draw() {
            if (shader != null) {
                shader.Use();

                GL.BindVertexArray(VAO);
                if (isSelected)
                    GL.Uniform4(shader.GetUniformLocation("inputColor"), Color4.Green.R, Color4.Green.G, Color4.Green.B, 1.0f);
                else
                    GL.Uniform4(shader.GetUniformLocation("inputColor"), Color4.Red.R, Color4.Red.G, Color4.Red.B, 1.0f);
                GL.PointSize(15.0f);
                GL.DrawArrays(PrimitiveType.Points, 0, 1);
            }
        }

        public void SelectPoint(System.Numerics.Vector2 mousePos) {
            if (!isSelected) {
                System.Numerics.Vector2 pointPos = new(coordinates[0], coordinates[1]);
                if (System.Numerics.Vector2.Distance(mousePos, pointPos) < 0.1f) {
                    isSelected = true;;
                }
            } else {
                if (coordinates[0] * mousePos.X > 0) {
                    coordinates[0] = mousePos.X;
                }
                UpdateVertexBuffer();
            }
        }

        public void DeselectPoint() {
            if (isSelected) {
                isSelected = false;
            }
        }

        private void UpdateVertexBuffer() {
            GL.BindBuffer(BufferTarget.ArrayBuffer, VAO);
            GL.BufferData(BufferTarget.ArrayBuffer, coordinates.Length * sizeof(float), coordinates, BufferUsageHint.DynamicDraw);
        }

        public Vector3 GetCoordinates() {
            return new Vector3(coordinates[0], coordinates[1], 0f);
        }
    }
}