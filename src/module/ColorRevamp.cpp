module;

#include <opencv2/opencv.hpp>
#include <resource.h>
#include <filesystem>

module ColorRevamp;
import std;
import helper;

namespace nprivate
{
	namespace fs = std::filesystem;

	hlp::scalar select_color(const std::string& imagePath);
	hlp::scalar prepare_color(const hlp::__ids ids);
	void processImagesInDirectory(const fs::path& directory, const cv::Scalar& old_col, const cv::Scalar& new_col);

}

namespace color_revamp 
{
	void run()
	{
		const auto old_col_hls{ nprivate::prepare_color(ids_path_select_img1) };
		const auto new_col_hls{ nprivate::prepare_color(ids_path_select_img2) };

		nprivate::processImagesInDirectory("../media", old_col_hls, new_col_hls);
	}
}
