#include <cstdio>
#include <memory>
#include <map>

#include <snmy.h>
#include <blob.h>
#include <argparse.h>

static void disp_usage();

int main(int argc,const char* argv[]) {
	auto argparser = CArgParser(argc,argv);

	if(!argparser.has_arguments()) {
		disp_usage();
		std::exit(-1);
	}

	/* read command-line inputs -------------------------*/
	bool do_showusage = false;
	bool do_verbose = false;

	std::string param_srcfile;
	std::string param_outfile;
	std::string param_action;

	if(argparser.arg_isValid("--help")) {
		do_showusage = true;
	}
	if(argparser.arg_isValid("-a",1)) {
		param_action = argparser.arg_get("-a",1)[1];
	}
	if(argparser.arg_isValid("-i",1)) {
		param_srcfile = argparser.arg_get("-i",1)[1];
	}
	if(argparser.arg_isValid("-o",1)) {
		param_outfile = argparser.arg_get("-o",1)[1];
	}
	if(argparser.arg_isValid("-v")) {
		do_verbose = true;
	}

	if(do_showusage) {
		disp_usage();
		std::exit(0);
	}

	if(param_srcfile.empty()) {
		std::puts("snmy: error: no source file specified");
		disp_usage();
		std::exit(-1);
	}
	if(param_outfile.empty()) {
		std::puts("snmy: error: no output file specified");
		disp_usage();
		std::exit(-1);
	}
	if(param_action.empty()) {
		std::puts("snmy: error: no action specified");
		disp_usage();
		std::exit(-1);
	}

	std::printf("out file: %s\n",param_outfile.c_str());
	std::printf("src file: %s\n",param_srcfile.c_str());

	/* convert ------------------------------------------*/
	std::map<std::string,bool> list_actions = {
		{"compress",true},
		{"decompress",true}
	};
	if(list_actions.count(param_action) == 0) {
		std::puts("snmy: error: invalid action");
		std::exit(-1);
	}

	if(param_action == "compress") {
		auto compressinfo = snmy::CCompressInfo();
		compressinfo.verbose_set(do_verbose);

		auto buffer = snmy::compress_file(param_srcfile,&compressinfo);
		buffer.send_file(param_outfile);
	} else if(param_action == "decompress") {
		auto buffer = snmy::decompress_file(param_srcfile);
		buffer.send_file(param_outfile);
	}
}

static void disp_usage() {
	//auto version ver = snmy::version_get();

	std::puts(
		"snmy -i <source_file> <options>\n"
		"usage:\n"
		"\t--help            show this dialog\n"
		"\t-a <action>       action to do (compress or decompress)\n"
		"\t-o <output_file>  specify output filename\n"
		"\t-i <source_file>  specify source filename\n"
		"\t-v                verbose output\n"
		"\texample: 'snmy -a compress -i file.bmp -o file.dat'"
	);
	//std::printf("\tsnmy compressor ver. %s\n",version.build_date.c_str());
};


