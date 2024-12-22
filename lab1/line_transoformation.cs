// using OpenTK.Windowing.Common;
// using OpenTK.Windowing.Desktop;
// using OpenTK.Graphics.OpenGL4;
// using OpenTK.Mathematics;
// using System;

// class Program
// {
//     private static float angle = 0.0f;
//     private static float scale = 1.0f;
//     private static Vector2 position = Vector2.Zero;
//     private static int windowWidth = 800;
//     private static int windowHeight = 800;

//     private static int shaderProgram;
//     private static int vao;

//     static void Main(string[] args)
//     {
//         var settings = new GameWindowSettings()
//         {
//             UpdateFrequency = 60.0
//         };

//         var nativeSettings = new NativeWindowSettings()
//         {
//             ClientSize = new Vector2i(windowWidth, windowHeight),
//             Title = "OpenGL Line Transformation"
//         };

//         using (var window = new GameWindow(settings, nativeSettings))
//         {
//             window.Load += OnLoad;
//             window.RenderFrame += (FrameEventArgs e) => OnRenderFrame(e, window);
//             window.UpdateFrame += (FrameEventArgs e) => OnUpdateFrame(e, window);
//             window.Run();
//         }
//     }

//     private static void OnLoad()
//     {
//         GL.ClearColor(0.0f, 0.0f, 0.0f, 1.0f);

//         // Создание шейдеров
//         string vertexShaderSource = @"
//             #version 330 core
//             layout(location = 0) in vec2 aPosition;
//             uniform mat4 transform;
//             void main()
//             {
//                 gl_Position = transform * vec4(aPosition, 0.0, 1.0);
//             }
//         ";
//         string fragmentShaderSource = @"
//             #version 330 core
//             out vec4 FragColor;
//             void main()
//             {
//                 FragColor = vec4(1.0, 1.0, 1.0, 1.0);
//             }
//         ";

//         int vertexShader = GL.CreateShader(ShaderType.VertexShader);
//         GL.ShaderSource(vertexShader, vertexShaderSource);
//         GL.CompileShader(vertexShader);

//         int fragmentShader = GL.CreateShader(ShaderType.FragmentShader);
//         GL.ShaderSource(fragmentShader, fragmentShaderSource);
//         GL.CompileShader(fragmentShader);

//         shaderProgram = GL.CreateProgram();
//         GL.AttachShader(shaderProgram, vertexShader);
//         GL.AttachShader(shaderProgram, fragmentShader);
//         GL.LinkProgram(shaderProgram);

//         GL.DeleteShader(vertexShader);
//         GL.DeleteShader(fragmentShader);

//         // Создание VAO и VBO
//         float[] vertices = { -0.5f, 0.0f, 0.5f, 0.0f };
//         int vbo = GL.GenBuffer();
//         vao = GL.GenVertexArray();

//         GL.BindVertexArray(vao);
//         GL.BindBuffer(BufferTarget.ArrayBuffer, vbo);
//         GL.BufferData(BufferTarget.ArrayBuffer, vertices.Length * sizeof(float), vertices, BufferUsageHint.StaticDraw);

//         GL.VertexAttribPointer(0, 2, VertexAttribPointerType.Float, false, 2 * sizeof(float), 0);
//         GL.EnableVertexAttribArray(0);

//         GL.BindBuffer(BufferTarget.ArrayBuffer, 0);
//         GL.BindVertexArray(0);
//     }

//     private static void OnRenderFrame(FrameEventArgs e, GameWindow window)
//     {
//         GL.Clear(ClearBufferMask.ColorBufferBit);

//         GL.UseProgram(shaderProgram);

//         // Матрица трансформации
//         Matrix4 transform = Matrix4.CreateTranslation(position.X, position.Y, 0.0f) *
//                             Matrix4.CreateScale(scale, scale, 1.0f) *
//                             Matrix4.CreateRotationZ(MathHelper.DegreesToRadians(angle));

//         int transformLoc = GL.GetUniformLocation(shaderProgram, "transform");
//         GL.UniformMatrix4(transformLoc, false, ref transform);

//         GL.BindVertexArray(vao);
//         GL.DrawArrays(PrimitiveType.Lines, 0, 2);

//         GL.BindVertexArray(0);
//         GL.UseProgram(0);

//         // SwapBuffers вызывается у самого экземпляра окна
//         window.SwapBuffers();
//     }

//     private static void OnUpdateFrame(FrameEventArgs e, GameWindow window)
//     {
//         var keyboardState = window.KeyboardState;

//         if (keyboardState.IsKeyDown(OpenTK.Windowing.GraphicsLibraryFramework.Keys.Up))
//         {
//             position.Y += 0.01f;
//         }
//         if (keyboardState.IsKeyDown(OpenTK.Windowing.GraphicsLibraryFramework.Keys.Down))
//         {
//             position.Y -= 0.01f;
//         }
//         if (keyboardState.IsKeyDown(OpenTK.Windowing.GraphicsLibraryFramework.Keys.Left))
//         {
//             position.X -= 0.01f;
//         }
//         if (keyboardState.IsKeyDown(OpenTK.Windowing.GraphicsLibraryFramework.Keys.Right))
//         {
//             position.X += 0.01f;
//         }
//         if (keyboardState.IsKeyDown(OpenTK.Windowing.GraphicsLibraryFramework.Keys.W))
//         {
//             scale += 0.01f;
//         }
//         if (keyboardState.IsKeyDown(OpenTK.Windowing.GraphicsLibraryFramework.Keys.S))
//         {
//             scale = Math.Max(scale - 0.01f, 0.1f);
//         }
//         if (keyboardState.IsKeyDown(OpenTK.Windowing.GraphicsLibraryFramework.Keys.A))
//         {
//             angle += 1.0f;
//         }
//         if (keyboardState.IsKeyDown(OpenTK.Windowing.GraphicsLibraryFramework.Keys.D))
//         {
//             angle -= 1.0f;
//         }
//     }
// }
