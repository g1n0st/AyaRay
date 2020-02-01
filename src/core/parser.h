#ifndef AYA_PHASER_H
#define AYA_PHASER_H

#include <vector>
#include <string>

#include "config.h"
#include "scene.h"

#include "../shapes/rectangle.h"
#include "../shapes/sphere.h"
#include "../shapes/heart.h"
#include "../shapes/triangle.h"
#include "../accelerators/BVH.h"

namespace Aya {
	/**@brief Parser class loads the JSON format scene description file to a scene class instance */
	class Parser {
	public:
		/**@brief JSON string after removing invalid characters */
		std::string m_str;
		/**@brief Pointer to the currently resolved position */
		int m_p;
		
		/**@brief instance cache of Transforms */
		std::vector<Transform *> m_trans;
		std::vector<Transform *> m_invts;
		/**@brief instance cache of Shapes */
		std::vector<SharedPtr<Shape> > m_shapes;
		/**@brief instance cache of Materials */
		std::vector<SharedPtr<Material> > m_materials;
		/**@brief instance cache of Primitives */
		std::vector<SharedPtr<Primitive> > m_prims;
		/**@brief instance cache of Camera */
		SharedPtr<Camera> m_cam;
		/**@brief screen_x, screen_y, sample_times loaded from JSON */
		int m_sx, m_sy, m_st;

		/**@brief Judge load the camera or not */
		bool rd_cam;
	public:
		Parser();
		
		/**@brief Load JSON from file name */
		void load(const char *file);
		/**@brief Parsing JSON to scene */
		void run(Scene *scene);

	private:
		/**@brief m_p++ and determine if the end of the string is read */
		inline void ADD();
		/**@brief Return the crruent symbol now parsing */
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

		/**@brief Load TriangleMesh from .obj file with UV data. */
		void loadObjc(const char *file, int &ts, int &vs, int **vv, Point3 **pp, Normal3 ** nn, float **uv);
		/**@brief Load TriangleMesh from .obj file. */
		void loadObj(const char *file, int &ts, int &vs, int **vv, Point3 **pp, Normal3 **nn);
		/**@brief Load TriangleMesh from .obj file without normal data. */
		void loadObjs(const char *file, int &ts, int &vs, int **vv, Point3 **pp);
		
		// read texture function group
		inline Texture * READ_CONSTANT_TEXTURE();
		inline Texture * READ_CROSS_TEXTURE();
		inline Texture * READ_NOISE_TEXTURE();
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