/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "DNA_mesh_types.h"
#include "DNA_meshdata_types.h"

#include "BKE_material.h"
#include "BKE_mesh.h"

#include "UI_interface.h"
#include "UI_resources.h"

#include "node_geometry_util.hh"

#include "../../extern/tinyexpr/tinyexpr.h"

static bNodeSocketTemplate geo_node_mesh_primitive_parametrics_in[] = {
    {SOCK_FLOAT, N_("Start u"), 0.0f, 0.0f, 0.0f, 0.0f, 0, FLT_MAX},
    {SOCK_FLOAT, N_("End u"), 6.28318f, 0.0f, 0.0f, 0.0f, 0, FLT_MAX},
    {SOCK_FLOAT, N_("Step u"), 0.1f, 0.0f, 0.0f, 0.0f, 0, FLT_MAX},
    {SOCK_STRING, N_("Slope u")},
    {SOCK_FLOAT, N_("Start v"), 0.0f, 0.0f, 0.0f, 0.0f, 0, FLT_MAX},
    {SOCK_FLOAT, N_("End v"), 6.28318f, 0.0f, 0.0f, 0.0f, 0, FLT_MAX},
    {SOCK_FLOAT, N_("Step v"), 0.1f, 0.0f, 0.0f, 0.0f, 0, FLT_MAX},
    {SOCK_STRING, N_("Slope v")},
    {SOCK_STRING, N_("Formula X")},
    {SOCK_STRING, N_("Formula Y")},
    {SOCK_STRING, N_("Formula Z")},
    {-1, ""},
};

static bNodeSocketTemplate geo_node_mesh_primitive_parametrics_out[] = {
    {SOCK_GEOMETRY, N_("Geometry")},
    {-1, ""},
};

static void geo_node_mesh_primitive_parametrics_layout(uiLayout *layout,
                                                    bContext *UNUSED(C),
                                                    PointerRNA *ptr)
{
  uiLayoutSetPropSep(layout, true);
  uiLayoutSetPropDecorate(layout, false);
  //uiItemR(layout, ptr, "dimentions_mode", 0, IFACE_("Dimentions Mode"), ICON_NONE);
}

static void geo_node_mesh_primitive_parametrics_init(bNodeTree *UNUSED(ntree), bNode *node)
{
  NodeGeometryMeshParametrics *node_storage = (NodeGeometryMeshParametrics *)MEM_callocN(
      sizeof(NodeGeometryMeshParametrics), __func__);

  node->storage = node_storage;
}

namespace blender::nodes {

static void fill_edge_data(MutableSpan<MEdge> edges)
{
  for (const int i : edges.index_range()) {
    edges[i].v1 = i;
    edges[i].v2 = i + 1;
    edges[i].flag |= ME_LOOSEEDGE;
  }
}

static Mesh *create_parametrics_curve(const double start_u,
                                      const double end_u,
                                      const double step_u,
                                      const std::string slope_u,
                                      const std::string formula_x,
                                      const std::string formula_y,
                                      const std::string formula_z)
{
  if (fabs(step_u) < FLT_EPSILON ||  // null step, avoid div by zero at nest line
      formula_x.empty() ||           // bad formula x
      formula_y.empty() ||           // bad formula y
      formula_z.empty())             // bad formula z
    return nullptr;

  const int verts_len = ceil((end_u - start_u) / step_u);
  if (verts_len == 0)
    return nullptr;

  Mesh *mesh = BKE_mesh_new_nomain(verts_len, verts_len - 1, 0, 0, 0);
  if (!mesh)
    return nullptr;

  BKE_id_material_eval_ensure_default_slot(&mesh->id);
  MutableSpan<MVert> verts{mesh->mvert, mesh->totvert};  // vertex
  MutableSpan<MEdge> edges{mesh->medge, mesh->totedge};

  double u;
  int err_x, err_y, err_z;
  te_variable vars[] = {{"t", &u}};
  te_expr *expr_x = te_compile(formula_x.c_str(), vars, 1, &err_x);
  te_expr *expr_y = te_compile(formula_y.c_str(), vars, 1, &err_y);
  te_expr *expr_z = te_compile(formula_z.c_str(), vars, 1, &err_z);

  short normal[3];
  normal_float_to_short_v3(normal, float3(0, 0, 1.0f));  // parce que plan xy

  double step_u_f = (end_u - start_u) / (double)(verts_len - 1);  // -1 for loop connection
  u = (double)start_u;

  float3 pos;

  for (int verts_index = 0; verts_index < verts_len; verts_index++) {
    if (expr_x)
      pos.x = (float)te_eval(expr_x);
    else
      pos.x = 0.0f;

    if (expr_y)
      pos.y = (float)te_eval(expr_y);
    else
      pos.y = 0.0f;

    if (expr_z)
      pos.z = (float)te_eval(expr_z);
    else
      pos.z = 0.0f;

    // printf("p : %.5f, %.5f\n", px, py);

    copy_v3_v3(verts[verts_index].co, pos);
    copy_v3_v3_short(verts[verts_index].no, normal);

    u += step_u_f;
  }

  if (!expr_x)
    printf("Parse error for params x at %d\n", err_x);
  if (!expr_y)
    printf("Parse error for params y at %d\n", err_y);
  if (!expr_z)
    printf("Parse error for params z at %d\n", err_z);

  te_free(expr_x);
  te_free(expr_y);
  te_free(expr_z);

  fill_edge_data(edges);
  BLI_assert(BKE_mesh_is_valid(mesh));

  return mesh;
}

static float3 evalPoint(te_expr** expr)
{
  float3 p(0.0f);

  if (expr[0])
    p.x = (float)te_eval(expr[0]);

  if (expr[1])
    p.y = (float)te_eval(expr[1]);

  if (expr[2])
    p.z = (float)te_eval(expr[2]);

  return p;
}

/*
==== X ====
y == 0
x == 0 => 4
x == l => 5
x > l => 3
4 5 3 3 3 3 3 3 3 3
y > 0
x == 0 => 3
x == l => 4
x > l => 2
3 4 2 2 2 2 2 2 2 2
*/
static int getEdgeX(const int x, const int max_x, const int y)
{
  if (y == 0) {
    if (x == 1)
      return 4;
    else if (x == (max_x - 1))
      return 5;
    else
      return 3;
  }
  else {
    if (x == 1)
      return 3;
    else if (x == (max_x - 1))
      return 4;
    else
      return 2;
  }
}

/*
==== Y ====
y == 0
x == 0 => 4
x == 1 => 3
x > 1 => 3
4 3 3 3 3 3 3 3 3 3
y > 0
x == 0 => 3
x == 1 => 2
x > 1 => 2
3 2 2 2 2 2 2 2 2 2
*/
static int getEdgeY(const int x, const int y)
{
  if (y == 1) {
    if (x == 0)
      return 2;
    else if (x == 1)
      return 4;
    else
      return 3;
  }
  else {
    if (x == 1)
      return 3;
    else
      return 2;
  }
}

static Mesh *create_parametrics_surface(const double start_u,
                                        const double end_u,
                                        const double step_u,
                                        const std::string slope_u,
                                        const double start_v,
                                        const double end_v,
                                        const double step_v,
                                        const std::string slope_v,
                                        const std::string formula_x,
                                        const std::string formula_y,
                                        const std::string formula_z)
{
  //return nullptr;

  if (fabs(step_u) < FLT_EPSILON ||  // null step, avoid div by zero at nest line
      fabs(step_v) < FLT_EPSILON ||  // null step, avoid div by zero at nest line
      formula_x.empty() ||           // bad formula x
      formula_y.empty() ||            // bad formula y
      formula_z.empty())              // bad formula z
    return nullptr;

  double u_ref, v_ref;
  int err_x, err_y, err_z;
  te_variable vars[] = {{"u", &u_ref}, {"v", &v_ref}};
  te_expr* expr[3];
  expr[0] = te_compile(formula_x.c_str(), vars, 2, &err_x);
  expr[1] = te_compile(formula_y.c_str(), vars, 2, &err_y);
  expr[2] = te_compile(formula_z.c_str(), vars, 2, &err_z);

  int count_activated = 0;
  if (expr[0])
    count_activated++;
  if (expr[1])
    count_activated++;
  if (expr[2])
    count_activated++;

  if (count_activated >=2) {
    const int verts_len_u = ceil((end_u - start_u) / step_u);
    const int verts_len_v = ceil((end_v - start_v) / step_v);
    const int verts_len = verts_len_u * verts_len_v;
    if (verts_len == 0)
      return nullptr;

    const int polys_len_u = verts_len_u - 1;
    const int polys_len_v = verts_len_v - 1;
    const int polys_len = polys_len_u * polys_len_v;

    if (polys_len == 0)
      return nullptr;

    const int loops_len = polys_len * 4;
    const int edges_len = 4 + 3 * (polys_len_u - 1) + 3 * (polys_len_v - 1) +
                          2 * (polys_len_u - 1) * (polys_len_v - 1);

    //printf("Count Verts : %i\n", verts_len);
    //printf("Count Polys : %i\n", polys_len);
    //printf("Count Loops : %i\n", loops_len);
    //printf("Count Edges : %i\n", edges_len);

    Mesh *mesh = BKE_mesh_new_nomain(verts_len, edges_len, 0, loops_len, polys_len);
    if (!mesh)
      return nullptr;

    BKE_id_material_eval_ensure_default_slot(&mesh->id);
    MutableSpan<MVert> verts{mesh->mvert, mesh->totvert};  // vertex
    MutableSpan<MLoop> loops{mesh->mloop, mesh->totloop};  // face vertexs
    MutableSpan<MPoly> polys{mesh->mpoly, mesh->totpoly};  // face
    MutableSpan<MEdge> edges{mesh->medge, mesh->totedge};  // face edges
  
    double step_u_f = (end_u - start_u) / (double)(verts_len_u - 1);  // -1 for loop connection
    u_ref = (double)start_u;
    double step_v_f = (end_v - start_v) / (double)(verts_len_v - 1);  // -1 for loop connection
    v_ref = (double)start_v;

    int poly_index = 0;
    int verts_index = 0;
    int loops_index = 0;
    int edges_index = 0;

    int edge_v1_v2_accum = 1;
    int edge_v0_v1_accum = 0;

    float3 pos;
    for (int verts_index_v = 0; verts_index_v < verts_len_v; verts_index_v++) {
      v_ref = start_v + verts_index_v * step_v_f;
      for (int verts_index_u = 0; verts_index_u < verts_len_u; verts_index_u++) {
        u_ref = start_u + verts_index_u * step_u_f;

        pos = evalPoint(expr);
        copy_v3_v3(verts[verts_index].co, pos);

        if ((verts_index_u < polys_len_u) && (verts_index_v < polys_len_v)) {
          //printf("Poly : %i ---\n", poly_index);
          
          MPoly &poly = polys[poly_index++];
          poly.loopstart = loops_index;
          poly.totloop = 4;

          int v0 = verts_index;
          int v1 = v0 + 1;
          int v2 = v1 + verts_len_u;
          int v3 = v2 - 1;

          /*
           v03 -- v02
            |      |
            |      |
           v00 -- v01
          */

          loops[loops_index].v = v0;
          if (verts_index_v == 0) {
            loops[loops_index].e = edges_index;
          }
          else {
            edge_v0_v1_accum += getEdgeY(verts_index_u, verts_index_v);
            loops[loops_index].e = edge_v0_v1_accum;
          }
          //printf("Edge : %i\n", loops[loops_index].e);
          loops_index++;

          if (verts_index_v == 0) {
            auto edge = &edges[edges_index];
            edge->v1 = v0;
            edge->v2 = v1;
            edge->flag = ME_EDGEDRAW | ME_EDGERENDER;
            edges_index++;
          }

          loops[loops_index].v = v1;
          loops[loops_index].e = edges_index;
          //printf("Edge : %i\n", loops[loops_index].e);
          loops_index++;
          auto edge = &edges[edges_index];
          edge->v1 = v1;
          edge->v2 = v2;
          edge->flag = ME_EDGEDRAW | ME_EDGERENDER;
          edges_index++;

          loops[loops_index].v = v2;
          loops[loops_index].e = edges_index;
          //printf("Edge : %i\n", loops[loops_index].e);
          loops_index++;
          edge = &edges[edges_index];
          edge->v1 = v2;
          edge->v2 = v3;
          edge->flag = ME_EDGEDRAW | ME_EDGERENDER;
          edges_index++;

          loops[loops_index].v = v3;
          if (verts_index_u == 0) {
            loops[loops_index].e = edges_index;
          }
          else {
            loops[loops_index].e = edge_v1_v2_accum;
            edge_v1_v2_accum += getEdgeX(verts_index_u, verts_len_u - 1, verts_index_v);
          }
          //printf("Edge : %i\n", loops[loops_index].e);
          loops_index++;

          if (verts_index_u == 0) {
            edge = &edges[edges_index];
            edge->v1 = v3;
            edge->v2 = v0;
            edge->flag = ME_EDGEDRAW | ME_EDGERENDER;
            edges_index++;
          }
        }
       
        verts_index++;
      }
    }

    if (!expr[0])
      printf("Parse error for params x at %d\n", err_x);
    if (!expr[1])
      printf("Parse error for params y at %d\n", err_y);
    if (!expr[2])
      printf("Parse error for params z at %d\n", err_z);

    te_free(expr[0]);
    te_free(expr[1]);
    te_free(expr[2]);

    BKE_mesh_calc_normals(mesh);
    BLI_assert(BKE_mesh_is_valid(mesh));

    return mesh;
  }

  // here can be only one
  // but its a fail since we must mesh a surface
  // so 2 params mini

  te_free(expr[0]);
  te_free(expr[1]);
  te_free(expr[2]);
   
  return nullptr;
}

static void geo_node_mesh_primitive_parametrics_update(bNodeTree *UNUSED(tree), bNode *node)
{

}

static void geo_node_mesh_primitive_parametrics_exec(GeoNodeExecParams params)
{
  const bNode &node = params.node();
  const NodeGeometryMeshParametrics &storage = *(const NodeGeometryMeshParametrics *)node.storage;

  const float start_u = params.extract_input<float>("Start u");
  const float end_u = params.extract_input<float>("End u");
  const float step_u = params.extract_input<float>("Step u");
  const std::string slope_u = params.extract_input<std::string>("Slope u");

  const float start_v = params.extract_input<float>("Start v");
  const float end_v = params.extract_input<float>("End v");
  const float step_v = params.extract_input<float>("Step v");
  const std::string slope_v = params.extract_input<std::string>("Slope v");

  const std::string formula_x = params.get_input<std::string>("Formula X");
  const std::string formula_y = params.get_input<std::string>("Formula Y");
  const std::string formula_z = params.get_input<std::string>("Formula Z");

   Mesh *mesh = create_parametrics_surface(start_u,
                                          end_u,
                                          step_u,
                                          slope_u,
                                          start_v,
                                          end_v,
                                          step_v,
                                          slope_v,
                                          formula_x,
                                          formula_y,
                                          formula_z);
   params.set_output("Geometry", GeometrySet::create_with_mesh(mesh));
}

}  // namespace blender::nodes

void register_node_type_geo_mesh_primitive_parametrics()
{
  static bNodeType ntype;

  geo_node_type_base(
      &ntype, GEO_NODE_MESH_PRIMITIVE_PARAMETRICS, "Parametrics", NODE_CLASS_GEOMETRY, 0);
  node_type_socket_templates(
      &ntype, geo_node_mesh_primitive_parametrics_in, geo_node_mesh_primitive_parametrics_out);
  node_type_storage(
      &ntype, "NodeGeometryMeshParametrics", node_free_standard_storage, node_copy_standard_storage);
  node_type_size(&ntype, 200, 120, 700);
  node_type_init(&ntype, geo_node_mesh_primitive_parametrics_init);
  node_type_update(&ntype, blender::nodes::geo_node_mesh_primitive_parametrics_update);
  ntype.geometry_node_execute = blender::nodes::geo_node_mesh_primitive_parametrics_exec;
  ntype.draw_buttons = geo_node_mesh_primitive_parametrics_layout;
  nodeRegisterType(&ntype);
}
