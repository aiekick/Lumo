	size_t Geometry::GetMiddlePoint_Icosaedre(const size_t& p1, const size_t& p2, std::vector<Vertex>* vVertices, std::map<std::tuple<size_t, size_t>, size_t>* vCache, const float& vRadius)
	size_t Geometry::GetMiddlePoint_Plane(const size_t& p1, const size_t& p2, std::vector<Vertex>* vVertices, std::map<std::tuple<size_t, size_t>, size_t>* vCache, const ct::fvec3& vNormal)
	Geometry::Geometry() = default;
	Geometry::~Geometry() = default;
	void Geometry::AddVertex(const ct::fvec3& vP, const ct::fvec3& vTarget)
	void Geometry::CreateGeometry(const bool& vGenerateCellNetWork)
	void Geometry::CreateGeometryAsync(const bool& vGenerateCellNetWork) // version thread�
	void Geometry::CreateIcosaedre(const float& vRadius, const int& vSubdivLevel, const bool& vGenerateCellNetWork)
		AddFace(0, 11, 5, &faces, vGenerateCellNetWork); //-V112
		AddFace(0, 5, 1, &faces, vGenerateCellNetWork); //-V112
		AddFace(0, 1, 7, &faces, vGenerateCellNetWork); //-V112
		AddFace(0, 7, 10, &faces, vGenerateCellNetWork); //-V112
		AddFace(0, 10, 11, &faces, vGenerateCellNetWork); //-V112
		AddFace(1, 5, 9, &faces, vGenerateCellNetWork); //-V112
		AddFace(5, 11, 4, &faces, vGenerateCellNetWork); //-V112
		AddFace(11, 10, 2, &faces, vGenerateCellNetWork); //-V112
		AddFace(10, 7, 6, &faces, vGenerateCellNetWork); //-V112
		AddFace(7, 1, 8, &faces, vGenerateCellNetWork); //-V112
		AddFace(3, 9, 4, &faces, vGenerateCellNetWork); //-V112
		AddFace(3, 4, 2, &faces, vGenerateCellNetWork); //-V112
		AddFace(3, 2, 6, &faces, vGenerateCellNetWork); //-V112
		AddFace(3, 6, 8, &faces, vGenerateCellNetWork); //-V112
		AddFace(3, 8, 9, &faces, vGenerateCellNetWork); //-V112
		AddFace(4, 9, 5, &faces, vGenerateCellNetWork); //-V112
		AddFace(2, 4, 11, &faces, vGenerateCellNetWork); //-V112
		AddFace(6, 2, 10, &faces, vGenerateCellNetWork); //-V112
		AddFace(8, 6, 7, &faces, vGenerateCellNetWork); //-V112
		AddFace(9, 8, 1, &faces, vGenerateCellNetWork); //-V112
	void Geometry::CreatePlane(const ct::fvec3& vOrigin, const ct::fvec3& vTarget, const ct::fvec2& vSize, const int& vSubdivLevel, const bool& vGenerateCellNetWork)
	void Geometry::Subdivide(const int& vSubdivLevel, const bool& vGenerateCellNetWork)
	void Geometry::AddFace(const size_t& vV0, const size_t& vV1, const size_t& vV2, std::vector<Face>* vFaces, const bool& vGenerateCellNetWork)
	void Geometry::AddEdge(const size_t& vV0, const size_t& vV1)
		ct::fvec3 e3; // empty fvec3
		ct::fvec2 e2; // empty fvec2
		for (auto& v : vertices)
			res.vertexs.emplace_back(VertexStruct::P3_N3_TA3_BTA3_T2_C4(v.p, v.n, e3, e3, e2, v.c));
		for (auto& f : faces)
		{
			res.indexs.push_back(VertexStruct::I1(f.v1));
			res.indexs.push_back(VertexStruct::I1(f.v2));
			res.indexs.push_back(VertexStruct::I1(f.v3));
			VertexStruct::P3_N3_TA3_BTA3_T2_C4& v0 = res.vertexs.at(f.v1);
			VertexStruct::P3_N3_TA3_BTA3_T2_C4& v1 = res.vertexs.at(f.v2);
			VertexStruct::P3_N3_TA3_BTA3_T2_C4& v2 = res.vertexs.at(f.v3);
			CalcNormal(v0, v1, v2);
		}
		for (auto& v : res.vertexs)
		{
			v.n = v.n.GetNormalized();
	void Geometry::AddTimeFrameToFile(const float& vTimeFrame, const std::string& vPrefix, const int& vFrame)
	void Geometry::SaveToObjFile(const std::string& vPrefix, const int& vFrame)
	MeshFrameStruct	Geometry::GetMeshFrame(const int& vFrame)



	bool Geometry::DrawDialogsAndPopups(const ct::ivec2& /*vScreenSize*/)