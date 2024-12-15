#include <opencv2/opencv.hpp>
#include <iostream>
#include <print>
#include <algorithm>
#include <locale>
#include <Windows.h>
#include <filesystem>
#include "RemoveColor.h"

import ColorRevamp;


using namespace cv;
using namespace std;
namespace fs = std::filesystem;

void RemoveLogo(cv::Mat& src);

Scalar convert_to_HSV(const Vec3b& col)
{
	Mat img{ 1,1, CV_8UC3, col };
	//	// Конвертация RGB в BGR
	//	Mat rgbMat(1, 1, CV_8UC3, rgb_color);

	const auto ncol = img.at<Vec3b>(0, 0);
	return ncol;
}

Scalar BGR_to_HSV(const cv::Scalar& bgr) {
	// Создаем матрицу BGR
	cv::Mat bgrMat(1, 1, CV_8UC3, cv::Scalar(bgr[0], bgr[1], bgr[2]));
	cv::Mat hsvMat;

	// Конвертируем BGR в HSV
	cv::cvtColor(bgrMat, hsvMat, cv::COLOR_BGR2HSV);

	// Получаем HSV значение
	cv::Vec3b hsvValue = hsvMat.at<cv::Vec3b>(0, 0);
	return cv::Scalar(hsvValue[0], hsvValue[1], hsvValue[2]);
}

void replaceColorWithMask(Mat& image, const Scalar& colorToReplace, const Scalar& newColor)
{
	// Преобразование изображения в цветовое пространство HSV
	Mat hsvImage;
	cvtColor(image, hsvImage, COLOR_BGR2HSV);

	const auto tolerance_col{ 5 };
	// Определение границ цвета для замены
	//Scalar lowerBound(colorToReplace[0] - tolerance_col, colorToReplace[1] - 50, colorToReplace[2] - 50);  // Допуск по цвету
	Scalar lowerBound(colorToReplace[0] - tolerance_col, 80, 50);  // Допуск по цвету
	Scalar upperBound(colorToReplace[0] + tolerance_col, 255, 255);

	// Создание маски для замены цветового диапазона
	Mat mask;
	inRange(hsvImage, lowerBound, upperBound, mask);

	// Применение новой цветовой тональности
	for (int y = 0; y < image.rows; y++)
	{
		for (int x = 0; x < image.cols; x++)
		{
			if (mask.at<uchar>(y, x) > 0)
			{ // Если пиксель попадает в маску
				Vec3b& pixel = hsvImage.at<Vec3b>(y, x);
				pixel[0] = newColor[0];
				pixel[1] = newColor[1];
				pixel[2] = newColor[2];

				//auto s = colorToReplace[1] >= pixel[1] ? colorToReplace[1] - pixel[1] : pixel[1] - colorToReplace[1];
				//auto v = colorToReplace[2] >= pixel[2] ? colorToReplace[2] - pixel[2] : pixel[2] - colorToReplace[2];
				//const auto maxa{ 10 };
				//s = s >= maxa ? maxa : s;
				//v = v >= maxa ? maxa : v;

				//const auto sa = colorToReplace[1] >= pixel[1] ? 1 : -1;
				//const auto va = colorToReplace[2] >= pixel[2] ? 1 : -1;

				//auto tmp_pixel{ pixel };
				//tmp_pixel[0] = newColor[0];
				//tmp_pixel[1] = newColor[1] + (sa * s);
				//tmp_pixel[2] = newColor[2] + (va * v);

				//const Vec3b end{ 50,50,50 };
				//const Vec3b start{ 245,245,245 };
				//if (
				//	tmp_pixel[1] > end[1] &&
				//	tmp_pixel[2] > end[2] &&
				//	tmp_pixel[1] < start[1] &&
				//	tmp_pixel[2] < start[2]
				//	)
				//{

				//	pixel = tmp_pixel;
				//}
				//else
				//{
				//	pixel[0] = tmp_pixel[0];
				//	pixel[1] = newColor[1];
				//	pixel[2] = newColor[2];
				//}
			}
		}
	}


	for (int y = 0; y < image.rows; y++)
	{
		for (int x = 0; x < image.cols; x++)
		{

			auto& pixel{ image.at<Vec3b>(y,x) };
			if (pixel[0] >= colorToReplace[0] - tolerance_col * 10 &&
				pixel[0] <= colorToReplace[0] + tolerance_col * 10 &&
				pixel[2] <= 60)
			{
				pixel[0] = newColor[0];
				pixel[1] = newColor[1];
				pixel[2] = newColor[2];
			}

			if (pixel[0] >= newColor[0] - tolerance_col &&
				pixel[0] <= newColor[0] + tolerance_col &&
				pixel[2] <= 60)
			{
				pixel[0] = newColor[0];
				pixel[1] = newColor[1];
				pixel[2] = newColor[2];
			}

		}
	}

	// Перевод обратно в BGR пространство
	cvtColor(hsvImage, image, COLOR_HSV2BGR);
}

// Функция для выбора цвета
Scalar chooseColor(const string& imagePath) {
	Mat image = imread(imagePath);
	if (image.empty()) {
		cerr << "Не удалось загрузить изображение: " << imagePath << endl;
		return Scalar(0, 0, 0); // Возвращаем черный цвет в случае ошибки
	}

	namedWindow("Выбор цвета", WINDOW_AUTOSIZE);
	imshow("Выбор цвета", image);

	// Ожидаем, пока пользователь кликнет на изображении
	Point selectedPoint;
	setMouseCallback("Выбор цвета", [](int event, int x, int y, int, void* userdata) {
		if (event == EVENT_MOUSEMOVE)
		{
			Point* pt = static_cast<Point*>(userdata);
			pt->x = x;
			pt->y = y;
		}
		else if (event == EVENT_LBUTTONDOWN) {
			Point* pt = static_cast<Point*>(userdata);
			pt->x = x;
			pt->y = y;
		}
		}, &selectedPoint);

	// Ждем, пока пользователь нажмет клавишу
	waitKey(0);
	Scalar color = image.at<Vec3b>(selectedPoint); // Получаем цвет из выбранной точки
	destroyAllWindows(); // Закрываем все окна
	return color; // Возвращаем выбранный цвет
}


void processImagesInDirectory(const fs::path& directory, const cv::Scalar& old_col, const cv::Scalar& new_col) {
	for (const auto& entry : fs::directory_iterator(directory)) {
		if (entry.is_directory()) {
			// Рекурсивно обрабатываем подпапки
			processImagesInDirectory(entry.path(), old_col, new_col);
		}
		else if (entry.is_regular_file()) {
			// Проверяем расширение файла
			auto extension = entry.path().extension().string();
			if (extension == ".jpg" || extension == ".jpeg" || extension == ".png") {
				cv::Mat image = cv::imread(entry.path().string());
				if (image.empty()) {
					std::cerr << "Ошибка при загрузке изображения: " << entry.path() << std::endl;
					continue;
				}

				RemoveLogo(image);
				replaceColorWithMask(image, old_col, new_col);

				// Сохранение изображения под тем же именем
				if (!cv::imwrite(entry.path().string(), image)) {
					std::cerr << "Ошибка при сохранении изображения: " << entry.path() << std::endl;
				}
				else {
					std::cout << "Изображение сохранено: " << entry.path() << std::endl;
				}
			}
		}
	}
}





int main() {
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	std::setlocale(LC_ALL, "Russian");

	color_revamp::run();

	const auto is_replace_color{ false };

	if (is_replace_color)
	{
		const Scalar color1 = chooseColor("../1.png"); // Замените на путь к первой картинке
		//color_revamp::print_color(color1);

		const Scalar color2 = chooseColor("../2.png"); // Замените на путь ко второй картинке
		//color_revamp::print_color(color2);


		//Scalar old_col(61, 38, 128); // Цвет, который нужно заменить (Белый)
		//Scalar new_col(239, 123, 0); // Цвет, который нужно заменить (Белый)

		const auto old_col_hlv{ BGR_to_HSV(color1) };
		const auto new_col_hlv{ BGR_to_HSV(color2) };

		//

		std::cout << "!!!!!!!!!!!!!old {}" << old_col_hlv << std::endl;
		std::cout << "!!!!!!!!!!!!!new {}" << new_col_hlv << std::endl;


		processImagesInDirectory("../media", old_col_hlv, new_col_hlv);
	}

	return 0;
}

void RemoveLogo(cv::Mat& src)
{
	const auto directory{ "../logo" };
	Mat newLogo = imread("../overlay.png"); // Новое изображение логотипа

	for (const auto& entry : fs::directory_iterator(directory))
	{
		if (entry.is_regular_file())
		{
			// Проверяем расширение файла
			auto extension = entry.path().extension().string();
			if (extension == ".jpg" || extension == ".jpeg" || extension == ".png")
			{
				std::println("путь: {}", entry.path().string());
				cv::Mat logo = cv::imread(entry.path().string());
				if (logo.empty()) {
					std::cerr << "Ошибка при загрузке изображения: " << entry.path() << std::endl;
					continue;
				}

				if (src.empty() || logo.empty() || newLogo.empty()) {
					cout << "Ошибка при загрузке изображений" << endl;
					return;
				}

				if (
					logo.size().width > src.size().width ||
					logo.size().height > src.size().height
					)
				{
					std::println("Изображение слишком маленькое!");
					continue;
				}
				// Сравнение размеров нового логотипа и старого логотипа
				if (newLogo.size() != logo.size()) {
					cout << "Изменение размера нового логотипа для соответствия старому" << endl;
					resize(newLogo, newLogo, logo.size());
				}

				// Выполнение шаблонного сопоставления
				Mat result;
				matchTemplate(src, logo, result, TM_CCOEFF_NORMED);

				// Пороговое значение для обнаружения схожести
				double minVal, maxVal;
				Point minLoc, maxLoc;
				minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);


				// Устанавливаем порог для идентификации позиции логотипа
				double threshold = 0.6; // Пороговое значение схожести
				std::print("maxValue {}", maxVal);

				if (maxVal >= threshold) {
					// Находим и заменяем логотип на новый
					Rect logoRect(maxLoc, logo.size());


					//const auto pick_color{ src.at<Vec3b>(logoRect.x, logoRect.y) };
					//Mat pick_mat{ 1, 1, CV_8UC3,  pick_color };
					//resize(pick_mat, pick_mat, logo.size());
					//pick_mat.copyTo(src(logoRect));
					newLogo.copyTo(src(logoRect));

					break;
				}
				else
				{
					cout << "Логотип не найден" << endl;
					continue;
				}
			}
		}
	}
}

