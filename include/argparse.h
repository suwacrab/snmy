#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <vector>
#include <string>

class CArgParser {
	public:
		static const int ARG_INVALID = -1;
	private:
		std::vector<std::string> m_arglist;
	public:
		CArgParser();
		CArgParser(int argc, const char* argv[]);

		auto arg_find(std::string name,int len = 0) -> int;
		auto arg_isValid(std::string name, int len = 0) -> bool;
		auto arg_get(std::string name, int len = 0) -> std::vector<std::string>;
		auto size() const -> int { return m_arglist.size(); }
		auto has_arguments() const -> bool { return size() > 0; }
};

#endif

