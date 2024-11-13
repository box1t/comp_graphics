using System;
using OpenTK.Graphics.OpenGL;
using OpenTK.Mathematics;
using System.Numerics;
using System.Collections.Generic;

namespace lab1 {
    public class Polyline {
        private readonly List<float> _controlPoints = [
            -0.5f, -0.5f, 0.0f,
            0.0f, 0.5f, 0.0f
        ];
        private int _selectedPoint = -1;
        private bool _isSelected = false;

        public bool _isInAddPointMode = false;

        int _vertexBufferObject;
        private int _vertexArrayObject;
        private Shader _shader;

        private float _animationTime = 0.0f;
        private float _animationSpeed = 1.0f;
        private bool _isAnimated = false;

        public void Load() {
            _vertexBufferObject = GL.GenBuffer();
            GL.BindBuffer(BufferTarget.ArrayBuffer, _vertexBufferObject);
            GL.BufferData(BufferTarget.ArrayBuffer, _controlPoints.Count * 3 * sizeof(float), _controlPoints.ToArray(), BufferUsageHint.StaticDraw);

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

        public void Draw() {
            _shader.Use();

            GL.BindVertexArray(_vertexArrayObject);
            GL.Uniform3(_shader.GetUniformLocation("inputColor"), Color4.White.R, Color4.White.G, Color4.White.B);
            GL.LineWidth(2.0f);
            GL.DrawArrays(PrimitiveType.LineStrip, 0, _controlPoints.Count / 3);

            for (int i = 0; i < _controlPoints.Count / 3; i++) {
                if (i == _selectedPoint) {
                    GL.PointSize(8.0f);
                    GL.Uniform3(_shader.GetUniformLocation("inputColor"), Color4.DarkRed.R, Color4.DarkRed.G, Color4.DarkRed.B);
                } else {
                    GL.PointSize(6.0f);
                    GL.Uniform3(_shader.GetUniformLocation("inputColor"), Color4.White.R, Color4.White.G, Color4.White.B);
                }

                GL.DrawArrays(PrimitiveType.Points, i, 1);
            }
        }

        public void TurnAnimation() {
            if (_isAnimated) {
                _isAnimated = false;
            }
            else 
                _isAnimated = true;
        }

        public void Animate(double time) {
            if (_isAnimated) {
                _animationTime += (float)time * _animationSpeed;
                UpdatePoints();
            }
        }

        public void IncreaseSpeed() {
            if (_isAnimated) _animationSpeed += 0.1f;
        }

        public void ReduceSpeed() {
            if (_isAnimated && _animationSpeed > 0) _animationSpeed -= 0.1f;
        }

        private void UpdatePoints() {
            for (int i = 0; i < _controlPoints.Count; i += 3) {
                _controlPoints[i + 1] = (float)Math.Sin(_animationTime + i) * 0.5f;
            }

            UpdateVertexBuffer();
        }

        public void SelectPoint(System.Numerics.Vector2 mousePos) {
            if (_isAnimated) _isAnimated = false;
            if (!_isSelected) {
                for (int i = 0; i < _controlPoints.Count / 3; i++) {
                    System.Numerics.Vector2 pointPos = new System.Numerics.Vector2(_controlPoints[i * 3], _controlPoints[i * 3 + 1]);
                    if (System.Numerics.Vector2.Distance(mousePos, pointPos) < 0.05f) {
                        _selectedPoint = i;
                        _isSelected = true;
                        break;
                    }
                }
            } else {
                _controlPoints[_selectedPoint * 3] = mousePos.X;
                _controlPoints[_selectedPoint * 3 + 1] = mousePos.Y;

                GL.BindBuffer(BufferTarget.ArrayBuffer, _vertexBufferObject);
                GL.BufferData(BufferTarget.ArrayBuffer, _controlPoints.Count * sizeof(float), _controlPoints.ToArray(), BufferUsageHint.DynamicDraw);
            }
        }

        public void DeselectPoint() {
            if (_isSelected) {
                _isSelected = false;
                _selectedPoint = -1;
            }
        }

        public void AddPoint(System.Numerics.Vector2 newPoint) {
            _controlPoints.Add(newPoint.X);
            _controlPoints.Add(newPoint.Y);
            _controlPoints.Add(0.0f);

            UpdateVertexBuffer();
        }

        public void DeletePoint() {
            if (_isSelected) {
                _controlPoints.RemoveRange(_selectedPoint * 3, 3);
                UpdateVertexBuffer();
                _selectedPoint = -1;
                _isSelected = false;
            }
        }
        public void DeleteLastPoint() {
            if (_controlPoints.Count > 0) {
                _controlPoints.RemoveRange(_controlPoints.Count - 3, 3);
                UpdateVertexBuffer();
            }
        }

        private void UpdateVertexBuffer() {
            GL.BindBuffer(BufferTarget.ArrayBuffer, _vertexBufferObject);
            GL.BufferData(BufferTarget.ArrayBuffer, _controlPoints.Count * sizeof(float), _controlPoints.ToArray(), BufferUsageHint.StaticDraw);
        }

        public void EnterAddPointMode() {
            _isInAddPointMode = true;
        } 
        public void ExitAddPointMode() {
            _isInAddPointMode = false;
        }
    }
}