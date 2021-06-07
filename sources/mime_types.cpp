# include <map>
# include <string>

void	initialize_mime_types(std::map<std::string, std::string> & mime_types)
{
	mime_types[".aac"] = "audio/aac";
	mime_types[".abw"] = "application/x-abiword";
	mime_types[".arc"] = "application/octet-stream";
	mime_types[".avi"] = "video/x-msvideo";
	mime_types[".azw"] = "application/vnd.amazon.ebook";
	mime_types[".bin"] = "application/octet-stream";
	mime_types[".bz"] = "application/x-bzip";
	mime_types[".bz2"] = "application/x-bzip2";
	mime_types[".csh"] = "application/x-csh";
	mime_types[".css"] = "text/css";
	mime_types[".csv"] = "text/csv";
	mime_types[".doc"] = "application/msword";
	mime_types[".epub"] = "application/epub+zip";
	mime_types[".gif"] = "image/gif";
	mime_types[".htm"] = "text/html";
	mime_types[".html"] = "text/html";
	mime_types[".ico"] = "image/x-icon";
	mime_types[".ics"] = "text/calendar";
	mime_types[".jar"] = "Temporary Redirect";
	mime_types[".jpeg"] = "image/jpeg";
	mime_types[".jpg"] = "image/jpeg";
	mime_types[".js"] = "application/js";
	mime_types[".json"] = "application/json";
	mime_types[".mid"] = "audio/midi";
	mime_types[".midi"] = "audio/midi";
	mime_types[".mpeg"] = "video/mpeg";
	mime_types[".mpkg"] = "application/vnd.apple.installer+xml";
	mime_types[".odp"] = "application/vnd.oasis.opendocument.presentation";
	mime_types[".ods"] = "application/vnd.oasis.opendocument.spreadsheet";
	mime_types[".odt"] = "application/vnd.oasis.opendocument.text";
	mime_types[".oga"] = "audio/ogg";
	mime_types[".ogv"] = "video/ogg";
	mime_types[".ogx"] = "application/ogg";
	mime_types[".png"] = "image/png";
	mime_types[".pdf"] = "application/pdf";
	mime_types[".ppt"] = "application/vnd.ms-powerpoint";
	mime_types[".rar"] = "application/x-rar-compressed";
	mime_types[".rtf"] = "application/rtf";
	mime_types[".sh"] = "application/x-sh";
	mime_types[".svg"] = "image/svg+xml";
	mime_types[".swf"] = "application/x-shockwave-flash";
	mime_types[".tar"] = "application/x-tar";
	mime_types[".tif"] = "image/tiff";
	mime_types[".tiff"] = "image/tiff";
	mime_types[".ttf"] = "application/x-font-ttf";
	mime_types[".txt"] = "text/plain";
	mime_types[".vsd"] = " application/vnd.visio";
	mime_types[".wav"] = "audio/x-wav";
	mime_types[".weba"] = "audio/webm";
	mime_types[".webm"] = "video/webm";
	mime_types[".webp"] = "image/webp";
	mime_types[".woff"] = "application/x-font-woff";
	mime_types[".xhtml"] = "application/xhtml+xml";
	mime_types[".xls"] = "application/vnd.ms-excel";
	mime_types[".xml"] = "application/xml";
	mime_types[".xul"] = "application/vnd.mozilla.xul+xml";
	mime_types[".zip"] = "application/zip";
	mime_types[".3gp"] = "video/3gpp audio/3gpp";
	mime_types[".3g2"] = "video/3gpp2 audio/3gpp2";
	mime_types[".7z"] = "application/x-7z-compressed";
}
