#include "parser.h"

namespace Aya {

	Parser::Parser() {
		rd_cam = false;
		m_sx = m_sy = m_st = false;
	}

	void Parser::load(const char *file) {
		FILE *fp = fopen(file, "r");
		assert(fp != NULL);

		char c;
		std::string str;
		while ((c = fgetc(fp)) != EOF) {
			if (c == '\n' || c == ' ' || c == '\t') continue;
			m_str += c;
		}
		std::cout << m_str << std::endl;
		fclose(fp);
	}
	void Parser::ADD() {
		assert((++m_p) < m_str.size());
	}
	char Parser::CUR() {
		return m_str[m_p];
	}
	void Parser::READ_BRACE_BEGIN() {
		assert(CUR() == '{'); ADD();
	}
	inline void Parser::READ_BRACE_END() {
		assert(CUR() == '}'); ADD();
	}
	inline void Parser::READ_COLON() {
		assert(CUR() == ':'); ADD();
	}
	inline void Parser::READ_ARRAY_BEGIN() {
		assert(CUR() == '[');  ADD();
	}
	inline void Parser::READ_ARRAY_END() {
		assert(CUR() == ']');  ADD();
	}
	inline bool Parser::READ_BRACE_ELEMENT_END() {
		if (CUR() == '}') return true;
		assert(CUR() == ',');  ADD();
		return false;
	}
	inline bool Parser::READ_ARRAY_ELEMENT_END() {
		if (CUR() == ']') return true;
		assert(CUR() == ','); ADD();
		return false;
	}
	inline std::string Parser::READ_STRING() {
		assert(CUR() == '\"');  ADD();
		std::string ret;
		do {
			ret += CUR();
			ADD();
		} while (CUR() != '\"');
		++m_p;
		return ret;
	}
	inline bool Parser::READ_BOOL() {
		if (CUR() == 't') { ADD();
		if (CUR() == 'r') { ADD();
			if (CUR() == 'u'); {ADD();
			if (CUR() == 'e') { ADD(); return true; }}}
			assert(0);
		}
		else if (CUR() == 'f') { ADD();
			if (CUR() == 'a') { ADD();
				if (CUR() == 'l') { ADD();
					if (CUR() == 's') { ADD();
						if (CUR() == 'e') { ADD(); return false; }}}}
			assert(0);
		}
		assert(0);
	}
	inline int Parser::READ_INT() {
		int ret = 0;
		while (CUR() >= '0' && CUR() <= '9') {
			ret = ret * 10 + (CUR() - '0'); ADD();
		}
		return ret;
	}
	inline float Parser::READ_FLOAT() {
		float ret = 0.f, decimal = 1.0f;
		bool read_dot = false;
		while ((CUR() >= '0' && CUR() <= '9') || CUR() == '.') {
			if (CUR() == '.') {
				assert(!read_dot); ADD();
				read_dot = true; 
				continue;
			}
			if (!read_dot) {
				ret = ret * 10 + (CUR() - '0'); ADD();
			}
			else {
				decimal *= .1f;
				ret += decimal * (CUR() - '0'); ADD();
			}
		}
		return ret;
	}
	inline BaseVector3 Parser::READ_VECTOR() {
		float x = 0, y = 0, z = 0;
		READ_BRACE_BEGIN();
		do {
			std::string index = READ_INDEX();
			if (index == "x") x = READ_FLOAT();
			if (index == "y") y = READ_FLOAT();
			if (index == "z") z = READ_FLOAT();
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
		return BaseVector3(x, y, z);
	}
	inline Spectrum Parser::READ_SPECTRUM1() {
		float r = 0, g = 0, b = 0;
		READ_BRACE_BEGIN();
		do {
			std::string index = READ_INDEX();
			if (index == "r") r = READ_FLOAT();
			else if (index == "g") g = READ_FLOAT();
			else if (index == "b") b = READ_FLOAT();
			else assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
		return Spectrum(r, g, b);
	}
	inline Spectrum Parser::READ_SPECTRUM256() {
		float r = 0, g = 0, b = 0;
		READ_BRACE_BEGIN();
		do {
			std::string index = READ_INDEX();
			if (index == "r") r = READ_FLOAT();
			else if (index == "g") g = READ_FLOAT();
			else if (index == "b") b = READ_FLOAT();
			else assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
		return Spectrum(r / 256.0, g / 256.0, b / 256.0);
	}

	inline std::string Parser::READ_INDEX() {
		std::string ret = READ_STRING();
		READ_COLON();
		return ret;
	}

	inline void Parser::READ_CONFIG() {
		READ_BRACE_BEGIN();
		do {
			std::string index = READ_INDEX();
			if (index == "screen_x") m_sx = READ_INT();
			if (index == "screen_y") m_sy = READ_INT();
			if (index == "sample_times") m_st = READ_INT();
			else assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
	}
	inline void Parser::READ_CAMERA() {
		READ_BRACE_BEGIN();
		BaseVector3 lookfrom, lookat, lookup;
		float fov, aspect, aperture = .0f, focus_dist, t0 = .0f, t1 = 0;
		bool l_lookfrom = false, l_lookat = false, l_lookup = false, l_fov = false, l_aspect = false, l_focus_dist = false;
		do {
			std::string index = READ_INDEX();
			if (index == "lookfrom") lookfrom = READ_VECTOR(), l_lookfrom = true;
			else if (index == "lookat") lookat = READ_VECTOR(), l_lookat = true;
			else if (index == "lookup") lookup = READ_VECTOR(), l_lookup = true;
			else if (index == "fov") fov = READ_FLOAT(), l_fov = true;
			else if (index == "aspect") aspect = READ_FLOAT(), l_aspect = true;
			else if (index == "aperture") aperture = READ_FLOAT();
			else if (index == "focus_dist") focus_dist = READ_FLOAT(), l_focus_dist = true;
			else if (index == "t0") t0 = READ_FLOAT();
			else if (index == "t1") t1 = READ_FLOAT();
			else assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
		assert(l_lookfrom && l_lookat && l_lookup && l_fov && l_aspect && l_focus_dist);
		m_cam = new ProjectiveCamera(lookfrom, lookat, lookup, fov, aspect, aperture, focus_dist, t0, t1);
		rd_cam = true;
	}
	void Parser::run(Scene *scene) {
		m_p = 0;
		if (m_str.size() == 0) {
			printf("Please load .json config file first!\n");
			assert(0);
		}
		READ_BRACE_BEGIN();
		do {
			std::string index = READ_INDEX();
			if (index == "config") READ_CONFIG();
			if (index == "camera") READ_CAMERA();
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
	}
}