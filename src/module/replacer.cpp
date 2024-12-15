module;

#include <opencv2/opencv.hpp>
#include <filesystem>
#include "resource.h"
#include <string_view>
#include <ranges>

module ColorRevamp;
import helper;


namespace nprivate
{
	struct max_value
	{
		double max_val{ 0.0 };
		hlp::point max_loc{};
	};

	namespace fs = std::filesystem;


	void replace_color_with_mask(hlp::mat& image, const hlp::scalar& colorToReplace, const hlp::scalar& newColor);


	//На основании образцов из папки Logo пытается найти на актуальном изображении(src) и
	//заменить его на логотип (overlay.png)
	void replace_logo(hlp::mat& src);

	//Сравнивает формат (extension) с дефолтными форматами
	bool check_format(const std::string& extension);

	bool find_and_replace(hlp::mat& src, hlp::mat& logo, hlp::mat& new_logo);
	max_value logo_min_max_loc(const hlp::mat& logo, const hlp::mat& logo_target, hlp::mat& new_logo);
	bool equally_size(const hlp::mat& img1, const hlp::mat& img2);
	bool is_image_larger_logo(const hlp::mat& logo, const hlp::mat& img);
	bool equally_format(const std::string_view& str1, const std::string_view& str2);
	cv::Vec3b& assign_vec3b_from_scalar(cv::Vec3b& v, const hlp::scalar& s);
	void copy_logo_to_rect_coord(const hlp::point& max_loc, const hlp::mat& new_logo, hlp::mat& src);
	//--------------------------------------------------



	void replace_color_with_mask(hlp::mat& image, const hlp::scalar& colorToReplace, const hlp::scalar& new_color)
	{
		// Преобразование изображения в цветовое пространство HSV
		hlp::mat hsv_image{};
		cv::cvtColor(image, hsv_image, cv::COLOR_BGR2HSV);

		const auto tcol{ hlp::load_string(ids_tcol_i) };
		const auto tolerance_col{ std::stod(tcol) };

		// Определение границ цвета для замены
		hlp::scalar lowerBound(colorToReplace[0] - tolerance_col, 80, 50);  // Допуск по цвету
		hlp::scalar upperBound(colorToReplace[0] + tolerance_col, 255, 255);

		// Создание маски для замены цветового диапазона
		hlp::mat mask;
		cv::inRange(hsv_image, lowerBound, upperBound, mask);

		// Применение новой цветовой тональности
		for (int y : std::views::iota(0, image.rows))
		{
			for (int x : std::views::iota(0, image.cols))
			{
				if (mask.at<uchar>(y, x) > 0)
				{ // Если пиксель попадает в маску
					auto& pixel{ hsv_image.at<cv::Vec3b>( y,x) };
					assign_vec3b_from_scalar(pixel, new_color);
				}
			}
		}

		for (int y : std::views::iota(0, image.rows))
		{
			for (int x : std::views::iota(0, image.cols))
			{
				auto& pixel{ image.at<cv::Vec3b>(y, x) };

				if (pixel[0] >= colorToReplace[0] - tolerance_col * 10 &&
					pixel[0] <= colorToReplace[0] + tolerance_col * 10 &&
					pixel[2] <= 60)
				{
					assign_vec3b_from_scalar(pixel, new_color);
				}

				if (pixel[0] >= new_color[0] - tolerance_col &&
					pixel[0] <= new_color[0] + tolerance_col &&
					pixel[2] <= 60)
				{
					assign_vec3b_from_scalar(pixel, new_color);
				}
			}
		}

		// Перевод обратно в BGR пространство
		cv::cvtColor(hsv_image, image, cv::COLOR_HSV2BGR);
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

					replace_logo(image);
					replace_color_with_mask(image, old_col, new_col);

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

	void replace_logo(hlp::mat& src)
	{
		if (!hlp::check_image(src)) return;

		hlp::mat new_logo{ hlp::load_image(ids_path_overlay) };
		if (!hlp::check_image(new_logo)) return;

		const auto directory_path{ hlp::load_string(ids_path_logo_templ).c_str() };

		for (const auto& entry : fs::directory_iterator(directory_path))
		{
			if (entry.is_regular_file())
			{
				const auto extension{ entry.path().extension().string() };
				if (!check_format(extension)) continue;

				hlp::mat logo{ hlp::load_image(entry.path().string()) };
				if (!hlp::check_image(logo)) continue;

				if (!is_image_larger_logo(logo, src))
				{
					hlp::print_message(ids_message_img_little);
					continue;
				}

				if (find_and_replace(src, logo, new_logo)) break;
			}
		}
	}

	bool check_format(const std::string& extension)
	{
		const auto fmt1{ hlp::load_string(ids_format1) };
		const auto fmt2{ hlp::load_string(ids_format2) };
		const auto fmt3{ hlp::load_string(ids_format3) };

		return equally_format(extension, fmt1) || equally_format(extension, fmt2) || equally_format(extension, fmt3);
	}

	bool find_and_replace(hlp::mat& src, hlp::mat& logo, hlp::mat& new_logo)
	{
		const auto [max_val, max_loc] = logo_min_max_loc(src, logo, new_logo);

		const auto tolerance_str{ hlp::load_string(ids_tolerance_d) };
		double threshold = std::stod(tolerance_str);

		if (max_val >= threshold) {
			copy_logo_to_rect_coord(max_loc, new_logo, src);
			return true;
		}
		else
		{
			hlp::print_message(ids_msg_logo_not_found);
			return false;
		}
	}

	max_value logo_min_max_loc(const hlp::mat& logo, const hlp::mat& logo_target, hlp::mat& new_logo)
	{
		if (!equally_size(new_logo, logo_target))
			cv::resize(new_logo, new_logo, logo_target.size());

		hlp::mat result;
		cv::matchTemplate(logo, logo_target, result, cv::TM_CCOEFF_NORMED);

		double min_val, max_val;
		hlp::point min_loc, max_loc;
		cv::minMaxLoc(result, &min_val, &max_val, &min_loc, &max_loc);

		return { max_val, max_loc };
	}

	void copy_logo_to_rect_coord(const hlp::point& max_loc, const hlp::mat& new_logo, hlp::mat& src)
	{
		cv::Rect logo_rect(max_loc, new_logo.size());
		new_logo.copyTo(src(logo_rect));
	}

	bool equally_size(const hlp::mat& img1, const hlp::mat& img2)
	{
		return img1.size() == img2.size();
	}

	bool is_image_larger_logo(const hlp::mat& logo, const hlp::mat& img)
	{
		return !(logo.size().width > img.size().width ||
			logo.size().height > img.size().height);
	}

	bool equally_format(const std::string_view& str1, const std::string_view& str2)
	{
		return str1 == str2;
	}

	cv::Vec3b& assign_vec3b_from_scalar(cv::Vec3b& v, const hlp::scalar& s)
	{
		v[0] = static_cast<uchar>(s[0]); // B
		v[1] = static_cast<uchar>(s[1]); // G
		v[2] = static_cast<uchar>(s[2]); // R
		return v;
	}
}
