#ifndef AYA_PHASER_H
#define AYA_PHASER_H

#include <vector>
#include <string>

#include "config.h"
#include "scene.h"

namespace Aya {
	class Parser {
	public:
		std::string m_str;
		int m_p;
		
		std::vector<Transform *> m_trans;
		std::vector<Transform *> m_invts;
		std::vector<SharedPtr<Shape> > m_shapes;
		std::vector<SharedPtr<Material> > m_materials;
		std::vector<SharedPtr<Primitive> > m_prims;
		SharedPtr<Camera> m_cam;
		int m_sx, m_sy, m_st;

		bool rd_cam;
	public:
		Parser();
		
		void load(const char *file);
		//void loadObj(const char *file, int &ts, int &vs, int *vv, Point3 *pp, Normal3 *nn);
		//void loadObjs(const char *file, int &ts, int &vs, int *vv, Point3 *pp);
		void run(Scene *scene);

	private:
		inline void ADD();
		inline char CUR();

		inline void READ_BRACE_BEGIN();
		inline void READ_BRACE_END();
		inline void READ_COLON();
		inline void READ_ARRAY_BEGIN();
		inline void READ_ARRAY_END();
		inline bool READ_BRACE_ELEMENT_END();
		inline bool READ_ARRAY_ELEMENT_END();

		inline std::string READ_STRING();
		inline bool READ_BOOL();
		inline int READ_INT();
		inline float READ_FLOAT();
		inline BaseVector3 READ_VECTOR();
		inline Spectrum READ_SPECTRUM1();
		inline Spectrum READ_SPECTRUM256();

		inline std::string READ_INDEX();

		inline void READ_CONFIG();
		inline void READ_CAMERA();
	};
}

#endif