#include "HTTP.hpp"
#include <dirent.h>

void HTTP::GET(std::string &fsPath)
{
	if (is_directory(fsPath))
	{
		buildResponse(200, request.best_location.fs_index);
		return ;
	}
	
	return buildResponse(200, fsPath);


	// Check if this is index.html and replace {{file_list}} placeholder
	// if (fsPath.find("index.html") != std::string::npos)
	// {
	// 	std::string fileListHtml = generateFileListHtml("./upload");
	// 	size_t pos = bodyContent.find("{{file_list}}");
	// 	if (pos != std::string::npos)
	// 	{
	// 		bodyContent.replace(pos, 14,
	// 							fileListHtml); // 14 = length of "{{file_list}}"
	// 	}
	// }

	// Set successful response

	// Ensure proper content disposition
	// if (mime.find("text/") == 0 || mime == "application/javascript")
	// {
	// 	response.headers["Content-Disposition"] = "inline";
	// }
}
