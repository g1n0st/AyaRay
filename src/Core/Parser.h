#ifndef AYA_CORE_PHASER_H
#define AYA_CORE_PHASER_H

#include "Config.h"
#include "Scene.h"

#include "../Shapes/Rectangle.h"
#include "../Shapes/Sphere.h"
#include "../Shapes/Heart.h"
#include "../Shapes/Triangle.h"
#include "../Accelerators/BVH.h"

#include <vector>
#include <string>

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
		void run(Scene *scene);

	private:
		inline void ADD();
		inline char CUR();

		// handle format functions

		inline void READ_BRACE_BEGIN();
		inline void READ_BRACE_END();
		inline void READ_COLON();
		inline void READ_ARRAY_BEGIN();
		inline void READ_ARRAY_END();
		inline bool READ_BRACE_ELEMENT_END();
		inline bool READ_ARRAY_ELEMENT_END();

		// read specify format functions

		inline std::string READ_STRING();
		inline bool READ_BOOL();
		inline int READ_INT();
		inline float READ_FLOAT();
		inline BaseVector3 READ_VECTOR();
		inline Spectrum READ_SPECTRUM1();
		inline Spectrum READ_SPECTRUM256();

		inline std::string READ_INDEX();

		// read config function group
		inline void READ_CONFIG();

		// read camara function group
		inline void READ_CAMERA();

		// read transform function group
		inline void READ_TRANSFORMS();
		inline Transform READ_TRANSFORM();
		inline Transform READ_TRANSLATE();
		inline Transform READ_SCALE();
		inline Transform READ_EULER_ZYX();
		inline Transform READ_EULER_YPR();
		inline Transform READ_ROTAION();
		inline Transform READ_ROTATEX();
		inline Transform READ_ROTATEY();
		inline Transform READ_ROTATEZ();

		// read shape function group
		inline void READ_SHAPES();
		inline void READ_SHAPE();
		inline void READ_HEART();
		inline void READ_RECTANGLE();
		inline void READ_SPHERE();
		inline void READ_TRIANGLE_MESH();

		void loadObjc(const char *file, int &ts, int &vs, int **vv, Point3 **pp, Normal3 ** nn, float **uv);
		void loadObj(const char *file, int &ts, int &vs, int **vv, Point3 **pp, Normal3 **nn);
		void loadObjs(const char *file, int &ts, int &vs, int **vv, Point3 **pp);
		
		// read texture function group
		inline Texture * READ_CONSTANT_TEXTURE();
		inline Texture * READ_CROSS_TEXTURE();
		inline Texture * READ_NOISE_TEXTURE();
		inline Texture * READ_IMAGE_TEXTURE();
		inline Texture * READ_TEXTURE();

		// read material function group
		inline void READ_MENTAL_MATERIAL();
		inline void READ_DIFFUSE_MATERIAL();
		inline void READ_LAMBERTIAN_MATERIAL();
		inline void READ_DIELECTRIC_MATERIAL();
		inline void READ_MATERIAL();
		inline void READ_MATERIALS();

		// read primitive function group
		inline void READ_PRIMITIVES();
		inline void READ_PRIMITIVE();
	};
}

#endif