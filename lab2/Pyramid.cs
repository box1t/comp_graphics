namespace lab2 {
    public class Pyramid : Shape {
        private static readonly float[] vertices = [
            // Base vertices
            -0.5f, -0.5f,  0.5f,  // Bottom-left
             0.5f, -0.5f,  0.5f,  // Bottom-right
            -0.5f, -0.5f, -0.5f,  // Top-left
             0.5f, -0.5f, -0.5f,  // Top-right

            // Apex of the pyramid (height)
            0.0f,  0.5f,  0.0f    // Increased height
        ];

        private static readonly uint[] indices = [
            // Front face
            0, 1, 4,
            // Back face
            2, 3, 4,
            // Left face
            0, 2, 4,
            // Right face
            1, 3, 4,
            // Base face
            0, 1, 2,
            1, 3, 2
        ];

        public Pyramid() : base(vertices, indices) {}
    }
}