using OpenTK.Graphics.OpenGL;
using OpenTK.Mathematics;

namespace lab2 {
    public class Shape {
        private float[] vertices = [];
        private uint[] indices = [];
        private Shader? shader;
        private int shapeVBO, shapeVAO, indicesVBO;
        private Matrix4 model, view, projection;
        private float prevCameraPos = 0f;
        public bool isSelected = false;
        private System.Numerics.Vector2 prevMousePos;

        public Shape() {}

        public Shape(float[] vertices,  uint[] indices) {
            this.vertices = vertices;
            this.indices = indices;
        }

        public void Initialize(float[] vertices, uint[] indices) {
            this.vertices = vertices;
            this.indices = indices;
        }

        private void LoadShapeVBO() {
            shapeVBO = GL.GenBuffer();
            GL.BindBuffer(BufferTarget.ArrayBuffer, shapeVBO);
            GL.BufferData(BufferTarget.ArrayBuffer, vertices.Length * sizeof(float), vertices, BufferUsageHint.DynamicDraw);
        }

        private void LoadShapeVAO() {
            shapeVAO = GL.GenVertexArray();
            GL.BindVertexArray(shapeVAO);
            GL.VertexAttribPointer(0, 3, VertexAttribPointerType.Float, false, 3 * sizeof(float), 0);
            GL.EnableVertexAttribArray(0);
        }

        private void LoadIndicesVBO() {
            indicesVBO = GL.GenBuffer();
            GL.BindBuffer(BufferTarget.ElementArrayBuffer, indicesVBO);
            GL.BufferData(BufferTarget.ElementArrayBuffer, indices.Length * sizeof(uint), indices, BufferUsageHint.DynamicDraw);
        }

        private void LoadPerspective(int width, int height) {
            if (shader != null) {
                projection = Matrix4.CreatePerspectiveFieldOfView(MathHelper.DegreesToRadians(45.0f), width / (float)height, 0.1f, 100f);
                int projectionLocation = GL.GetUniformLocation(shader.Handle, "projection");
                GL.UniformMatrix4(projectionLocation, false, ref projection);
            }
        }

        private void LoadView(Vector3 cameraPosition) {
            if (shader != null) {
                view = Matrix4.LookAt(cameraPosition, Vector3.Zero, Vector3.UnitY);
                int viewLocation = GL.GetUniformLocation(shader.Handle, "view");
                GL.UniformMatrix4(viewLocation, false, ref view);
            }
        }

        private void LoadModel() {
            if (shader != null) {
                model = Matrix4.Identity;
                int modelLocation = GL.GetUniformLocation(shader.Handle, "model");
                GL.UniformMatrix4(modelLocation, false, ref model);
            }
        }

        private void LoadShader() {
            shader = new Shader("shaders/shader.vert", "shaders/shader.frag");
            shader.Use();
        }

        public void Load(int width, int height) {
            LoadShapeVBO();
            LoadShapeVAO();
            LoadIndicesVBO();

            LoadShader();

            LoadView(new Vector3(0.0f, 0.0f, 3.0f));
            LoadModel();       
            LoadPerspective(width, height);
        }

        public void UpView() {
            if (shader != null) {
                shader.Use();
                if (prevCameraPos < 1.0f) prevCameraPos += 0.01f;
                var cameraPosition = new Vector3(0.0f, prevCameraPos, 3.0f);  // Камера на высоте с углом
                LoadView(cameraPosition);
            }
        }

        public void DownView() {
            if (shader != null) {
                shader.Use();
                if (prevCameraPos > -1.0f) prevCameraPos -= 0.01f;
                var cameraPosition = new Vector3(0.0f, prevCameraPos, 3.0f);  // Камера на высоте с углом
                LoadView(cameraPosition);
            }
        }

        public virtual Vector3 CalculateCenter() {
            float sumX = 0, sumY = 0, sumZ = 0;
            int vertexCount = vertices.Length / 3;

            for (int i = 0; i < vertices.Length; i += 3) {
                sumX += vertices[i];
                sumY += vertices[i + 1];
                sumZ += vertices[i + 2];
            }

            return new Vector3(sumX / vertexCount, sumY / vertexCount, sumZ / vertexCount);;
        }

        public void ChangePerspective(Vector3 firstPoint, Vector3 secondPoint) {
            if (shader != null) {
                Vector3 newCenter = CalculateCenter();
                float distanceToFirstPoint = Vector3.Distance(newCenter, firstPoint);
                float distanceToSecondPoint = Vector3.Distance(newCenter, secondPoint);
                if (MathHelper.Abs(distanceToSecondPoint) > MathHelper.Abs(distanceToFirstPoint)) {
                    (firstPoint, secondPoint) = (secondPoint, firstPoint);
                }
                shader.Use();
                Vector3 xDirection = Vector3.Normalize(firstPoint - new Vector3(0f, 0f, 3f)) * 1.5f;
                if (MathHelper.Abs(distanceToSecondPoint) > MathHelper.Abs(distanceToFirstPoint)) xDirection *= -1;
                Vector3 zDirection = Vector3.Normalize(secondPoint - new Vector3(0f, 0f, 3f)) * 1.5f;
                Vector3 yDirection = Vector3.UnitY;

                Vector3 center = CalculateCenter();

                Matrix4 rotationMatrix = new(
                    new Vector4(xDirection, 0f),
                    new Vector4(yDirection, 0f),
                    new Vector4(zDirection, 0f),
                    new Vector4(0.0f, 0.0f, 0.0f, 1.0f)
                );

                Matrix4 translationToCenter;
                translationToCenter = Matrix4.CreateTranslation(center);
                model = rotationMatrix * translationToCenter;

                int modelLocation = GL.GetUniformLocation(shader.Handle, "model");
                GL.UniformMatrix4(modelLocation, false, ref model);
            }
        }

        public void ChangeScale(float size) {
            for (int i = 0; i < vertices.Length; i += 3) {
                vertices[i] *= size;
                vertices[i + 1] *= size;
                vertices[i + 2] *= size;
            }
            UpdateVertexBuffer();
        }

        public virtual void Drag(System.Numerics.Vector3 translation) {
            for (int i = 0; i < vertices.Length; i += 3) {
                vertices[i] += translation.X;
            }
            UpdateVertexBuffer();
        }

        public virtual void Move(System.Numerics.Vector3 translation) {
            for (int i = 0; i < vertices.Length; i += 3) {
                vertices[i] += translation.X;
                vertices[i + 1] += translation.Y;
                vertices[i + 2] += translation.Z;
            }
            UpdateVertexBuffer();
        }

        private static System.Numerics.Vector3 GetWorldCoordinates(System.Numerics.Vector2 coordinates, Matrix4 viewMatrix, Matrix4 projectionMatrix) {
            // Преобразуем координаты в пространство клипа (NDC)
            Vector4 clipCoords = new Vector4(coordinates.X, coordinates.Y, -1.0f, 1.0f);

            // Преобразуем из пространства клипа в пространство камеры
            Vector4 eyeCoords = projectionMatrix * clipCoords;

            // Преобразуем из пространства камеры в мировые координаты
            Vector4 worldCoords = viewMatrix * eyeCoords;

            // Возвращаем результат как вектор 3D
            return new System.Numerics.Vector3(worldCoords.X, worldCoords.Y, worldCoords.Z);
        }

        public void Draw() {
            if (shader != null) {
                shader.Use();

                GL.BindVertexArray(shapeVAO);
                if (!isSelected) GL.Uniform4(shader.GetUniformLocation("inputColor"), Color4.GhostWhite.R, Color4.GhostWhite.G, Color4.GhostWhite.B, 0.5f);
                else GL.Uniform4(shader.GetUniformLocation("inputColor"), Color4.LightGray.R, Color4.LightGray.G, Color4.LightGray.B, 0.5f);
                GL.DrawElements(PrimitiveType.Triangles, indices.Length, DrawElementsType.UnsignedInt, 0);
            }
        }

        public void Update(int width, int height) {
            GL.Viewport(0, 0, width, height);
            if (shader != null) {
                projection = Matrix4.CreatePerspectiveFieldOfView(MathHelper.DegreesToRadians(45f), width / (float)height, 0.1f, 100f);
                int projectionLocation = GL.GetUniformLocation(shader.Handle, "projection");
                GL.UniformMatrix4(projectionLocation, false, ref projection);
            }

        }

        public void Unload() {
            if (shader != null) {
                GL.DeleteBuffer(shapeVBO);
                GL.DeleteBuffer(indicesVBO);
                GL.DeleteVertexArray(shapeVAO);
                GL.DeleteProgram(shader.Handle);
            }
        }

        protected void UpdateVertexBuffer() {
            GL.BindBuffer(BufferTarget.ArrayBuffer, shapeVBO);
            GL.BufferData(BufferTarget.ArrayBuffer, vertices.Length * sizeof(float), vertices, BufferUsageHint.DynamicDraw);
        }

        public void SelectPoint(System.Numerics.Vector2 mousePos) {
            System.Numerics.Vector3 worldMousePos = GetWorldCoordinates(mousePos, view, projection);
            System.Numerics.Vector2 newMousePos = new(worldMousePos.X, worldMousePos.Y);
            if (!isSelected) {
                Vector3 newCenter = CalculateCenter();
                System.Numerics.Vector2 pointPos = new(newCenter.X, newCenter.Y);
                if (System.Numerics.Vector2.Distance(newMousePos, pointPos) < 0.4f) {
                    prevMousePos = newMousePos;
                    isSelected = true;
                }
            }
            else {
                System.Numerics.Vector3 translation = new(newMousePos - prevMousePos, 0f);
                translation *= 0.1f;
                Drag(translation);
                if (MathHelper.Abs(CalculateCenter().X) > 2f) Drag(-(translation));
                UpdateVertexBuffer();
            }
        }

        public void DeselectPoint() {
            isSelected = false;
        }
    }
}