#include <argparse.h>

CArgParser::CArgParser() {
	m_arglist.clear();
}
CArgParser::CArgParser(int argc, const char* argv[]) {
	// the arguments to parse actually start at 1
	// argv[0] is the program's path
	m_arglist.clear();
	
	if(argc == 1) return;
	for(int i=1; i<argc; i++) {
		auto str = std::string(argv[i]);
		m_arglist.push_back(str);
	}
}

auto CArgParser::arg_find(std::string name, int len) -> int {
	for(int i=0; i<size(); i++) {
		if(m_arglist[i] == name) {
			if(i+len >= size()) return CArgParser::ARG_INVALID;
			return i;
		}
	}
	return CArgParser::ARG_INVALID;
}
auto CArgParser::arg_isValid(std::string name, int len) -> bool {
	return arg_find(name,len) != CArgParser::ARG_INVALID;
}
auto CArgParser::arg_get(std::string name, int len) -> std::vector<std::string> {
	auto idx = arg_find(name,len);
	if(idx == CArgParser::ARG_INVALID) {
		return {};
	}

	std::vector<std::string> data;
	int num_args = 1 + len;
	for(int i=0; i<num_args; i++) {
		data.push_back(m_arglist[idx+i]);
	}
	return data;
}

