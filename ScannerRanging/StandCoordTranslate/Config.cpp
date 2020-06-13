#include "Config.h"

using namespace std;

std::string& Config::trim(std::string& text)
{
	if (!text.empty())
	{
		text.erase(0, text.find_first_not_of(" \n\r\t"));
		text.erase(text.find_last_not_of(" \n\r\t") + 1);
	}
	return text;
}

std::string& Config::toupper(std::string& text)
{
	for (auto& ch : text)
		ch = std::toupper(ch);
	return text;
}

std::string& Config::tolower(std::string& text)
{
	for (auto& ch : text)
		ch = std::tolower(ch);
	return text;
}

Config::Config(const char* filename, std::string delimiter, std::string comment)
	: Config(std::string(filename), delimiter, comment)
{}

Config::Config(string filename, string delimiter,
			   string comment)
	: m_Delimiter(delimiter), m_Comment(comment)
{
	std::ifstream in(filename);

	if (!in)
		throw File_not_found(filename);
	in >> (*this);
}

Config::Config()
	: m_Delimiter(string(1, '=')), m_Comment(string(1, '#'))
{}

bool Config::KeyExists(const string& key) const
{
	return m_Contents.find(key) != m_Contents.end();
}

std::ostream& operator<<(std::ostream& os, const Config& cf)
{
	for (auto& pach : cf.m_Contents)
		os << pach.first << " " << cf.m_Delimiter << " " << pach.second << std::endl;
	return os;
}

void Config::Remove(const string& key)
{
	m_Contents.erase(m_Contents.find(key));
}

std::istream& operator>>(std::istream& is, Config& cf)
{
	typedef string::size_type pos;
	const string& delim = cf.m_Delimiter;  // separator
	const string& comm = cf.m_Comment;    // comment
	const pos skip = delim.length();        // length of separator

	string nextline = "";  // might need to read ahead to see where value ends

	while (is || nextline.length() > 0)
	{
		// Read an entire line at a time
		string line;
		if (nextline.length() > 0)
		{
			line = nextline;  // we read ahead; use it now
			nextline = "";
		}
		else
		{
			std::getline(is, line);
		}

		// Ignore comments
		line = line.substr(0, line.find(comm));

		// Parse the line if it contains a delimiter
		pos delimPos = line.find(delim);
		if (delimPos < string::npos)
		{
			// Extract the key
			string key = line.substr(0, delimPos);
			line.replace(0, delimPos + skip, "");

			// See if value continues on the next line
			// Stop at blank line, next line with a key, end of stream,
			// or end of file sentry
			bool terminate = false;
			while (!terminate && is)
			{
				std::getline(is, nextline);
				terminate = true;

				string nlcopy = nextline;
				Config::trim(nlcopy);
				if (nlcopy == "") continue;

				nextline = nextline.substr(0, nextline.find(comm));
				if (nextline.find(delim) != string::npos)
					continue;

				nlcopy = nextline;
				Config::trim(nlcopy);
				if (nlcopy != "") line += "\n";
				line += nextline;
				terminate = false;
			}

			// Store key and value
			Config::trim(key);
			Config::trim(line);
			cf.m_Contents[key] = line;  // overwrites if key is repeated
		}
	}

	return is;
}
bool Config::FileExist(std::string filename)
{
	std::ifstream in(filename);
	return in.is_open();
}

void Config::ReadFile(string filename, string delimiter,
					  string comment)
{
	m_Delimiter = delimiter;
	m_Comment = comment;
	std::ifstream in(filename);

	if (!in)
		throw File_not_found(filename);

	in >> (*this);
}

void Config::SaveFile(std::string filename)
{
	std::ofstream out(filename.c_str());
	out << *this;
	out.close();
}