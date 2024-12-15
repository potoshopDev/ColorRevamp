module;

#include <opencv2/opencv.hpp>
#include "resource.h"

module ColorRevamp;
import std;
import helper;

namespace nprivate
{
	//--------------------------------------------------
	void wait_user_select_pixel(const char* const name_window, hlp::point& selectedPoint);
	void set_data_point(void* userdata, int x, int y);
	//--------------------------------------------------

	hlp::scalar select_color(const std::string& image_path)
	{
		auto image{ hlp::load_image(image_path) };
		if (!hlp::check_image(image)) return hlp::scalar(0, 0, 0);

		const auto name_window{ hlp::show_window(ids_choose_image, image) };

		hlp::point selected_point;
		wait_user_select_pixel(name_window.c_str(), selected_point);

		hlp::wait_press_anykey();
		const cv::Scalar color = image.at<cv::Vec3b>(selected_point);
		cv::destroyAllWindows();
		return color;
	}

	hlp::scalar prepare_color(const hlp::__ids ids)
	{
		const auto path_img{ hlp::load_string(ids) };
		const auto color = nprivate::select_color(path_img.c_str());
		return hlp::BGR_to_HLV(color);
	}

	void wait_user_select_pixel(const char* const name_window, hlp::point& selectedPoint)
	{
		setMouseCallback(name_window, [](int event, int x, int y, int, void* userdata)
			{
				if (event == cv::EVENT_MOUSEMOVE || event == cv::EVENT_LBUTTONDOWN)
				{
					set_data_point(userdata, x, y);
				}
			}, &selectedPoint);
	}

	void set_data_point(void* userdata, int x, int y)
	{
		auto pt = static_cast<hlp::point*>(userdata);
		pt->x = x;
		pt->y = y;
	}
}
