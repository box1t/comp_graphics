using System;
using OpenTK.Graphics.OpenGL;
using OpenTK.Mathematics;
using System.Collections.Generic;

namespace lab2
{
    public class Cylinder : Shape {

        private readonly int segmentCount = 32;
        private readonly float radius = 0.3f;
        private readonly float height = 0.6f;

        private readonly List<float> cylVertices = [];
        private readonly List<uint> cylIndices = [];

        public Cylinder() {
            GenerateCylinder();
            Initialize([.. cylVertices], [.. cylIndices]);
        }

        public override void Move(System.Numerics.Vector3 translation) {
            for (int i = 0; i < cylVertices.Count; i += 3) {
                cylVertices[i] += translation.X;
                cylVertices[i + 1] += translation.Y;
                cylVertices[i + 2] += translation.Z;
            }
            Initialize([.. cylVertices], [.. cylIndices]);
            UpdateVertexBuffer();
        }

        private void GenerateCylinder() {
            for (int i = 0; i <= segmentCount; i++)
            {
                float angle = (float)i / segmentCount * MathHelper.TwoPi;
                float x = radius * MathF.Cos(angle);
                float z = radius * MathF.Sin(angle);

                // Нижнее основание
                cylVertices.Add(x);
                cylVertices.Add(-height / 2);
                cylVertices.Add(z);

                // Верхнее основание
                cylVertices.Add(x);
                cylVertices.Add(height / 2);
                cylVertices.Add(z);
            }

            // Генерация индексов для боковой поверхности
            for (int i = 0; i < segmentCount; i++)
            {
                cylIndices.Add((uint)(i * 2));
                cylIndices.Add((uint)((i + 1) % segmentCount * 2));
                cylIndices.Add((uint)(i * 2 + 1));

                cylIndices.Add((uint)((i + 1) % segmentCount * 2));
                cylIndices.Add((uint)((i + 1) % segmentCount * 2 + 1));
                cylIndices.Add((uint)(i * 2 + 1));
            }

            // Индексы для верхнего основания
            int topCenterIndex = (int)cylIndices[^3];
            for (int i = 0; i < segmentCount; i++)
            {
                cylIndices.Add((uint)(i * 2 + 1));
                cylIndices.Add((uint)((i + 1) % segmentCount * 2 + 1));
                cylIndices.Add((uint)topCenterIndex);
            }

            // Индексы для нижнего основания
            int bottomCenterIndex = (int)cylIndices[^3];
            for (int i = 0; i < segmentCount; i++)
            {
                cylIndices.Add((uint)(i * 2));
                cylIndices.Add((uint)((i + 1) % segmentCount * 2));
                cylIndices.Add((uint)bottomCenterIndex);
            }
        }
    }
}
