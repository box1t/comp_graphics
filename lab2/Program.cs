using OpenTK.Graphics.OpenGL4;
using OpenTK.Mathematics;
using OpenTK.Windowing.Common;
using OpenTK.Windowing.Desktop;
using OpenTK.Windowing.GraphicsLibraryFramework;
using System;
using System.Collections.Generic;

public class Object3D
{
    public Vector3 Position;
    public int Type; // 0: Cube, 1: Pyramid, 2: Cylinder
}

public class MainWindow : GameWindow
{
    private readonly List<Object3D> objects = new();
    private int shaderProgram;
    private int cubeVAO, pyramidVAO, cylinderVAO;
    private int cubeEBO, pyramidEBO, cylinderEBO;

    private Vector3 vanishingPoint1 = new Vector3(-4.0f, 0.0f, 0.0f);
    private Vector3 vanishingPoint2 = new Vector3(4.0f, 0.0f, 0.0f);


    private float horizontalAngle = 0.0f;
    private float verticalAngle = 0.0f;
    private float cameraDistance = 5.0f;
    private Vector3 cameraPosition = new Vector3(0, 0, 5);

    private Vector2 lastMousePosition;
    private bool isDraggingObject = false;

    private int selectedObjectIndex = 0;

    private bool rightMousePressed = false;


    private int activeVanishingPoint = 1; // 1 - vanishingPoint1, 2 - vanishingPoint2
    private double lastLogTime = 0;
    public MainWindow() : base(GameWindowSettings.Default, new NativeWindowSettings
    {
        ClientSize = new Vector2i(1200, 900),
        Title = "Two-Point Perspective"
    })
    {
    }

    private int vanishingPointsVAO;
    private int vanishingPointsVBO;
    protected override void OnLoad()
    {
        base.OnLoad();

        GL.ClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        GL.Enable(EnableCap.DepthTest);

        shaderProgram = CreateShaderProgram();
        GL.UseProgram(shaderProgram);

        InitializeGeometry();

        // Add objects to the scene
        objects.Add(new Object3D { Position = new Vector3(-1.5f, 0, 0), Type = 0 }); // Cube
        objects.Add(new Object3D { Position = new Vector3(1.5f, 0, 0), Type = 1 });  // Pyramid
        objects.Add(new Object3D { Position = new Vector3(0, 0, -1.5f), Type = 2 }); // Cylinder

        vanishingPointsVAO = GL.GenVertexArray();
        vanishingPointsVBO = GL.GenBuffer();

        GL.BindVertexArray(vanishingPointsVAO);
        GL.BindBuffer(BufferTarget.ArrayBuffer, vanishingPointsVBO);
        GL.BufferData(BufferTarget.ArrayBuffer, 2 * Vector3.SizeInBytes, IntPtr.Zero, BufferUsageHint.DynamicDraw);

        GL.VertexAttribPointer(0, 3, VertexAttribPointerType.Float, false, Vector3.SizeInBytes, 0);
        GL.EnableVertexAttribArray(0);

        GL.BindVertexArray(0);
    }

    private void InitializeGeometry() 
    {
                // Cube geometry
        float[] cubeVertices = {
            // Positions          // Colors
            -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.5f, 0.5f, 0.5f
        };

        uint[] cubeIndices = {
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4,
            0, 4, 7, 7, 3, 0,
            1, 5, 6, 6, 2, 1,
            3, 2, 6, 6, 7, 3,
            0, 1, 5, 5, 4, 0
        };

        cubeVAO = CreateVAO(cubeVertices, cubeIndices, out cubeEBO);

        // Pyramid geometry
        float[] pyramidVertices = {
            // Positions          // Colors
             0.0f,  0.5f,  0.0f,  1.0f, 0.0f, 0.0f, // Apex
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, // Base 1
             0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f, // Base 2
             0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f, // Base 3
            -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f  // Base 4
        };

        uint[] pyramidIndices = {
            0, 1, 2,
            0, 2, 3,
            0, 3, 4,
            0, 4, 1,
            1, 2, 3, 3, 4, 1
        };

        pyramidVAO = CreateVAO(pyramidVertices, pyramidIndices, out pyramidEBO);

        // Cylinder geometry (example: using triangle strip)
        const int cylinderSegments = 36;
        float[] cylinderVertices = new float[(cylinderSegments + 1) * 2 * 6];
        uint[] cylinderIndices = new uint[cylinderSegments * 6];

        for (int i = 0; i <= cylinderSegments; i++)
        {
            float angle = MathF.PI * 2 * i / cylinderSegments;
            float x = MathF.Cos(angle) * 0.5f;
            float z = MathF.Sin(angle) * 0.5f;

            // Bottom circle
            cylinderVertices[i * 12 + 0] = x;
            cylinderVertices[i * 12 + 1] = -0.5f;
            cylinderVertices[i * 12 + 2] = z;
            cylinderVertices[i * 12 + 3] = 1.0f;
            cylinderVertices[i * 12 + 4] = 0.0f;
            cylinderVertices[i * 12 + 5] = 0.0f;

            // Top circle
            cylinderVertices[i * 12 + 6] = x;
            cylinderVertices[i * 12 + 7] = 0.5f;
            cylinderVertices[i * 12 + 8] = z;
            cylinderVertices[i * 12 + 9] = 0.0f;
            cylinderVertices[i * 12 + 10] = 1.0f;
            cylinderVertices[i * 12 + 11] = 0.0f;

            if (i < cylinderSegments)
            {
                int index = i * 6;
                cylinderIndices[index + 0] = (uint)(i * 2);
                cylinderIndices[index + 1] = (uint)(i * 2 + 1);
                cylinderIndices[index + 2] = (uint)((i + 1) * 2);

                cylinderIndices[index + 3] = (uint)((i + 1) * 2);
                cylinderIndices[index + 4] = (uint)(i * 2 + 1);
                cylinderIndices[index + 5] = (uint)((i + 1) * 2 + 1);
            }
        }

        cylinderVAO = CreateVAO(cylinderVertices, cylinderIndices, out cylinderEBO);
    }

    private int CreateVAO(float[] vertices, uint[] indices, out int ebo)
    {
        int vao = GL.GenVertexArray();
        GL.BindVertexArray(vao);

        int vbo = GL.GenBuffer();
        GL.BindBuffer(BufferTarget.ArrayBuffer, vbo);
        GL.BufferData(BufferTarget.ArrayBuffer, vertices.Length * sizeof(float), vertices, BufferUsageHint.StaticDraw);

        ebo = GL.GenBuffer();
        GL.BindBuffer(BufferTarget.ElementArrayBuffer, ebo);
        GL.BufferData(BufferTarget.ElementArrayBuffer, indices.Length * sizeof(uint), indices, BufferUsageHint.StaticDraw);

        GL.VertexAttribPointer(0, 3, VertexAttribPointerType.Float, false, 6 * sizeof(float), 0);
        GL.EnableVertexAttribArray(0);
        GL.VertexAttribPointer(1, 3, VertexAttribPointerType.Float, false, 6 * sizeof(float), 3 * sizeof(float));
        GL.EnableVertexAttribArray(1);

        return vao;
    }

    private void CheckShaderCompilation(int shader, string type)
    {
        GL.GetShader(shader, ShaderParameter.CompileStatus, out int status);
        if (status == 0)
        {
            string infoLog = GL.GetShaderInfoLog(shader);
            throw new Exception($"{type} Shader compilation failed: {infoLog}");
        }
    }

    private int CreateShaderProgram()
    {
        string vertexShaderSource = @"#version 330 core
        layout(location = 0) in vec3 aPosition;
        layout(location = 1) in vec3 aColor;

        out vec3 vertexColor;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        uniform vec3 vanishingPoint1;
        uniform vec3 vanishingPoint2;

        void main()
        {
            vec3 dir1 = normalize(vanishingPoint1 - aPosition);
            vec3 dir2 = normalize(vanishingPoint2 - aPosition);

            vec3 projectedPos = aPosition + dir1 + dir2;
            gl_Position = projection * view * model * vec4(projectedPos, 1.0);
            vertexColor = aColor;
        }";

        string fragmentShaderSource = @"#version 330 core
        in vec3 vertexColor;
        out vec4 fragColor;

        void main()
        {
            fragColor = vec4(vertexColor, 1.0);
        }";

        int vertexShader = GL.CreateShader(ShaderType.VertexShader);
        GL.ShaderSource(vertexShader, vertexShaderSource);
        GL.CompileShader(vertexShader);
        CheckShaderCompilation(vertexShader, "Vertex");

        int fragmentShader = GL.CreateShader(ShaderType.FragmentShader);
        GL.ShaderSource(fragmentShader, fragmentShaderSource);
        GL.CompileShader(fragmentShader);
        CheckShaderCompilation(fragmentShader, "Fragment");

        int program = GL.CreateProgram();
        GL.AttachShader(program, vertexShader);
        GL.AttachShader(program, fragmentShader);
        GL.LinkProgram(program);

        GL.DeleteShader(vertexShader);
        GL.DeleteShader(fragmentShader);

        return program;
    }

    protected override void OnRenderFrame(FrameEventArgs args)
    {
        base.OnRenderFrame(args);

        GL.Clear(ClearBufferMask.ColorBufferBit | ClearBufferMask.DepthBufferBit);

        float x = MathF.Cos(verticalAngle) * MathF.Sin(horizontalAngle) * cameraDistance;
        float y = MathF.Sin(verticalAngle) * cameraDistance;
        float z = MathF.Cos(verticalAngle) * MathF.Cos(horizontalAngle) * cameraDistance;
        cameraPosition = new Vector3(x, y, z);

        Matrix4 view = Matrix4.LookAt(cameraPosition, Vector3.Zero, Vector3.UnitY);
        Matrix4 projection = Matrix4.CreatePerspectiveFieldOfView(MathHelper.DegreesToRadians(90), Size.X / (float)Size.Y, 0.1f, 100.0f);

        foreach (var obj in objects)
        {

            Matrix4 model = Matrix4.CreateTranslation(obj.Position);

            GL.UseProgram(shaderProgram);
            GL.UniformMatrix4(GL.GetUniformLocation(shaderProgram, "model"), false, ref model);
            GL.UniformMatrix4(GL.GetUniformLocation(shaderProgram, "view"), false, ref view);
            GL.UniformMatrix4(GL.GetUniformLocation(shaderProgram, "projection"), false, ref projection);

            GL.Uniform3(GL.GetUniformLocation(shaderProgram, "vanishingPoint1"), ref vanishingPoint1);
            GL.Uniform3(GL.GetUniformLocation(shaderProgram, "vanishingPoint2"), ref vanishingPoint2);

            if (obj.Type == 0) // Cube
            {
                GL.BindVertexArray(cubeVAO);
                GL.DrawElements(PrimitiveType.Triangles, 36, DrawElementsType.UnsignedInt, 0);
            }
            else if (obj.Type == 1) // Pyramid
            {
                GL.BindVertexArray(pyramidVAO);
                GL.DrawElements(PrimitiveType.Triangles, 18, DrawElementsType.UnsignedInt, 0);
            }
            else if (obj.Type == 2) // Cylinder
            {
                GL.BindVertexArray(cylinderVAO);
                GL.DrawElements(PrimitiveType.Triangles, 36 * 6, DrawElementsType.UnsignedInt, 0);
            }
        }

        RenderVanishingPoints(view, projection);
        SwapBuffers();
    }

    private void UpdateVanishingPoints()
    {
        Vector3[] vanishingPoints = { vanishingPoint1, vanishingPoint2 };

        GL.BindBuffer(BufferTarget.ArrayBuffer, vanishingPointsVBO);
        GL.BufferSubData(BufferTarget.ArrayBuffer, IntPtr.Zero, 2 * Vector3.SizeInBytes, vanishingPoints);
    }
    private void RenderVanishingPoints(Matrix4 view, Matrix4 projection)
{
    UpdateVanishingPoints(); // Обновление данных в VBO

    GL.BindVertexArray(vanishingPointsVAO);
    GL.UseProgram(shaderProgram);

    GL.UniformMatrix4(GL.GetUniformLocation(shaderProgram, "view"), false, ref view);
    GL.UniformMatrix4(GL.GetUniformLocation(shaderProgram, "projection"), false, ref projection);

    GL.PointSize(10.0f);
    GL.DrawArrays(PrimitiveType.Points, 0, 2);

    GL.BindVertexArray(0);
}

    protected override void OnUpdateFrame(FrameEventArgs args)
    {
        base.OnUpdateFrame(args);

        // Переключение между точками схода
        if (KeyboardState.IsKeyDown(Keys.D1)) activeVanishingPoint = 1;
        if (KeyboardState.IsKeyDown(Keys.D2)) activeVanishingPoint = 2;

        // Переключение активного объекта с помощью правой кнопки мыши
        if (MouseState.IsButtonDown(MouseButton.Right))
        {
            if (!rightMousePressed)
            {
                rightMousePressed = true;
                selectedObjectIndex = (selectedObjectIndex + 1) % objects.Count;
                Console.WriteLine($"Selected Object Index: {selectedObjectIndex}");
            }
        }
        else
        {
            rightMousePressed = false;
        }

        // Перемещение активной точки схода
        if (activeVanishingPoint == 1)
        {
            if (KeyboardState.IsKeyDown(Keys.W)) vanishingPoint1.Y += 0.005f;
            if (KeyboardState.IsKeyDown(Keys.S)) vanishingPoint1.Y -= 0.005f;
            if (KeyboardState.IsKeyDown(Keys.A)) vanishingPoint1.X -= 0.005f;
            if (KeyboardState.IsKeyDown(Keys.D)) vanishingPoint1.X += 0.005f;
        }
        else if (activeVanishingPoint == 2)
        {
            if (KeyboardState.IsKeyDown(Keys.W)) vanishingPoint2.Y += 0.005f;
            if (KeyboardState.IsKeyDown(Keys.S)) vanishingPoint2.Y -= 0.005f;
            if (KeyboardState.IsKeyDown(Keys.A)) vanishingPoint2.X -= 0.005f;
            if (KeyboardState.IsKeyDown(Keys.D)) vanishingPoint2.X += 0.005f;
        }


        if (KeyboardState.IsKeyDown(Keys.Left)) horizontalAngle -= 0.002f;
        if (KeyboardState.IsKeyDown(Keys.Right)) horizontalAngle += 0.002f;
        if (KeyboardState.IsKeyDown(Keys.Up)) verticalAngle = Math.Clamp(verticalAngle + 0.002f, -MathF.PI / 2, MathF.PI / 2);
        if (KeyboardState.IsKeyDown(Keys.Down)) verticalAngle = Math.Clamp(verticalAngle - 0.002f, -MathF.PI / 2, MathF.PI / 2);

        if (KeyboardState.IsKeyDown(Keys.Z)) cameraDistance = MathF.Max(cameraDistance - 0.005f, 1.0f);
        if (KeyboardState.IsKeyDown(Keys.X)) cameraDistance = MathF.Min(cameraDistance + 0.005f, 20.0f);

        // Перемещение объекта с помощью мыши (левая кнопка)
        if (MouseState.IsButtonDown(MouseButton.Left))
        {
            if (!isDraggingObject)
            {
                isDraggingObject = true;
                lastMousePosition = new Vector2(MouseState.Position.X, MouseState.Position.Y);
            }

            Vector2 currentMousePosition = new Vector2(MouseState.Position.X, MouseState.Position.Y);
            Vector2 delta = currentMousePosition - lastMousePosition;
            lastMousePosition = currentMousePosition;

            objects[selectedObjectIndex].Position.X += delta.X * 0.005f; // Уменьшена скорость перемещения
            objects[selectedObjectIndex].Position.Y -= delta.Y * 0.005f;

            if (GLFW.GetTime() - lastLogTime > 0.5) // Ограничение частоты вывода
            {
                Console.WriteLine($"Object {selectedObjectIndex} moved to: {objects[selectedObjectIndex].Position}");
                lastLogTime = GLFW.GetTime();
            }
        }
        else
        {
            isDraggingObject = false;
        }

        // Ограничение вывода в консоль (интервал 0.5 секунды)
        if (GLFW.GetTime() - lastLogTime > 0.5)
        {
            Console.WriteLine($"Active Point: {activeVanishingPoint}, VanishingPoint1: {vanishingPoint1}, VanishingPoint2: {vanishingPoint2}");
            lastLogTime = GLFW.GetTime();
        }
    }


    protected override void OnResize(ResizeEventArgs e)
    {
        base.OnResize(e);
        GL.Viewport(0, 0, Size.X, Size.Y);
    }

    protected override void OnUnload()
    {
        GL.DeleteVertexArray(cubeVAO);
        GL.DeleteVertexArray(pyramidVAO);
        GL.DeleteVertexArray(cylinderVAO);
        GL.DeleteBuffer(cubeEBO);
        GL.DeleteBuffer(pyramidEBO);
        GL.DeleteBuffer(cylinderEBO);
        GL.DeleteProgram(shaderProgram);

        base.OnUnload();
    }

    public static void Main()
    {
        using var window = new MainWindow();
        window.Run();
    }
}
