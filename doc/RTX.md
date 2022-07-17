/*
	Ray generation shaders (“raygen”) begin all ray tracing work.
	A raygen shader runs on a 2D grid of threads, much like a compute shader,
	and is the starting point for tracing rays into the scene.
	It is also responsible for writing the final output from the ray
	tracing algorithm out to memory.
	*/
	virtual std::string GetRayGenerationShaderCode(std::string& vOutShaderName);

	/*
	Intersection shaders implement arbitrary ray-primitive intersection algorithms.
	They are useful to allow applications to intersect rays with different kinds of
	primitives (e.g., spheres) that do not have built-in support.
	Triangle primitives have built-in support and don’t require an intersection shader.
	*/
	virtual std::string GetRayIntersectionShaderCode(std::string& vOutShaderName);

	/*
	Miss shaders are invoked when no intersection is found for a given ray.
	*/
	virtual std::string GetRayMissShaderCode(std::string& vOutShaderName);

	/*
	Hit shaders are invoked when a ray-primitive intersection is found.
	They are responsible for computing the interactions that happen at an intersection
	point (e.g., light-material interactions for graphics applications)
	and can spawn new rays as needed.

	There are two kinds of hit shaders:
	*/

	/*
	any-hit shaders are invoked on all intersections of a ray with scene primitives,
	in an arbitrary order, and can reject intersections in addition to computing shading data.
	*/
	virtual std::string GetRayAnyHitShaderCode(std::string& vOutShaderName);

	/*
	Closest-hit shaders invoke only on the closest intersection point along the ray.
	*/
	virtual std::string GetRayClosestHitShaderCode(std::string& vOutShaderName);
