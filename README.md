# Задание
Программа должна иметь CLI или GUI.
Программа должна реализовывать весь следующий функционал по обработке png-файла
Общие сведения
Формат картинки PNG (рекомендуем использовать библиотеку libpng)
без сжатия
файл всегда соответствует формату PNG
обратите внимание на выравнивание; мусорные данные, если их необходимо дописать в файл для выравнивания, должны быть нулями.
все поля стандартных PNG заголовков в выходном файле должны иметь те же значения что и во входном (разумеется кроме тех, которые должны быть изменены).

Программа должна реализовывать следующий функционал по обработке PNG-файла

(1) Рисование треугольника. Треугольник определяется
- Координатами его вершин
- Толщиной линий
- Цветом линий
- Треугольник может быть залит или нет
- цветом которым он залит, если пользователем выбран залитый

(2) Находит самый большой прямоугольник заданного цвета и перекрашивает его в другой цвет. Функционал определяется:
- Цветом, прямоугольник которого надо найти
- Цветом, в который надо его перекрасить

(3) Создать коллаж размера N*M из одного либо нескольких фото -- на выбор студента (либо оба варианта по желанию). В случае с одним изображением коллаж представляет собой это же самое изображение повторяющееся N*M раз.
- Количество изображений по “оси” Y
- Количество изображений по “оси” X
- Перечень изображений (если выбрана усложненная версия задания)
