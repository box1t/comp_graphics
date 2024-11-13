namespace lab2 {
    public class Cube : Shape {
        private static readonly float[] vertices = [
            -0.5f, -0.5f,  0.5f,  // Bottom-left
            0.5f, -0.5f,  0.5f,  // Bottom-right
            0.5f,  0.5f,  0.5f,  // Top-right
            -0.5f,  0.5f,  0.5f,  // Top-left

            // Back face
            -0.5f, -0.5f, -0.5f,  // Bottom-left
            0.5f, -0.5f, -0.5f,  // Bottom-right
            0.5f,  0.5f, -0.5f,  // Top-right
            -0.5f,  0.5f, -0.5f   // Top-left
        ];

        private static readonly uint[] indices = [
            // Front face
            0, 1, 2,
            2, 3, 0,

            // Back face
            4, 5, 6,
            6, 7, 4,

            // Left face
            4, 0, 3,
            3, 7, 4,

            // Right face
            1, 5, 6,
            6, 2, 1,

            // Top face
            3, 2, 6,
            6, 7, 3,

            // Bottom face
            0, 1, 5,
            5, 4, 0
        ];

        public Cube() : base(vertices, indices) {}
    }
}