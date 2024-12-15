module;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <opencv2/opencv.hpp>
#include "resource.h"
#include <print>

export module helper;

export namespace hlp
{
	//--------------------------------------------------
	using scalar = cv::Scalar;
	using mat = cv::Mat;
	using point = cv::Point;
	using __ids = const unsigned int;
	using __size_string = const unsigned int;

	std::string load_string(__ids id, __size_string  size = 256);
	const std::string show_window(__ids ids, hlp::mat& image);
	void wait_press_anykey();
	scalar BGR_to_HLV(const scalar& bgr);
	mat load_image(__ids ids);
	mat load_image(const std::string& image_path);
	//--------------------------------------------------

	template <typename T>
		T get_pixel(const hlp::mat& image, int y, int x)
	{
		return image.at<T>(y, x); // Шаблонная функция для доступа к пикселю
	}

	std::string load_string(__ids id, __size_string  size)
	{
		const auto h_inst{ GetModuleHandle(nullptr) };
		std::string str;
		str.resize(size);

		LoadString(h_inst, id, str.data(), size);

		return str;
	}

	void print_color(const scalar& col)
	{
		std::println("RGB: {}, {}, {}.", col[2], col[1], col[0]);
	};

	void print_message(hlp::__ids ids)
	{
		const auto message{ load_string(ids) };
		std::println("{}", message);
	}

	bool check_image(const mat& image)
	{
		if (image.empty())
		{
			const auto error_message{ load_string(ids_check_image) };
			std::println("{}", error_message);
			return false;
		}
		return true;
	}

	mat load_image(const std::string& image_path)
	{
		return 	cv::imread(image_path);
	}

	mat load_image(__ids ids)
	{
		const auto path{ load_string(ids) };
		return load_image(path);
	}

	const std::string show_window(__ids ids, hlp::mat& image)
	{
		const auto choose_color_str{ hlp::load_string(ids_choose_image).c_str()};
		//const auto choose_color_str{"Тест"};

		cv::namedWindow(choose_color_str, cv::WINDOW_AUTOSIZE);
		imshow(choose_color_str, image);

		return choose_color_str;
	}
	void wait_press_anykey()
	{
		cv::waitKey(0);
	}

	scalar BGR_to_HLV(const scalar& bgr)
	{
		mat bgrMat(1, 1, CV_8UC3, cv::Scalar(bgr[0], bgr[1], bgr[2]));
		mat hsvMat;

		cv::cvtColor(bgrMat, hsvMat, cv::COLOR_BGR2HSV);

		cv::Vec3b hsvValue = hsvMat.at<cv::Vec3b>(0, 0);
		return scalar(hsvValue[0], hsvValue[1], hsvValue[2]);
	}

}