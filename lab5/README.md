# Лабораторная работа 5: Трассировка лучей (Ray Tracing)
## **7 вариант**

## Цель лабораторной работы:

В этой лабораторной работе вы научитесь работать с техникой трассировки лучей для создания реалистичной 3D-графики. Вы реализуете алгоритм Ray Tracing, который позволяет рассчитывать физически корректные отражения, преломления, тени и свет в сцене. Лабораторная работа подводит к пониманию основ рендеринга, работающего с лучами света, а также к созданию реалистичных сцен.

## Требования:
- Реализуйте алгоритм трассировки лучей для отрисовки простой сцены, используя минимальный набор примитивов (сферы, плоскости и т.д.).
- Реализуйте базовые эффекты: отражения, тени и освещение.
- Трассировка должна быть реализована как на CPU, так и с возможной оптимизацией на GPU (опционально).
- Программа должна корректно отображать сцены в зависимости от выбранного задания.


## Вариант 7. Отражения и текстурированные поверхности
- Постройте сцену с двумя текстурированными плоскостями (стена и пол) и одной сферой.
- Реализуйте трассировку лучей с поддержкой текстурирования объектов.
- Включите отражения на сфере и учтите отраженные текстуры на её поверхности.
- Дополнительно: Реализуйте управление зеркальностью поверхности сферы для изменения интенсивности отражений.