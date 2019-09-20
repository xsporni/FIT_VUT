#include <student/gpu.h>

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <iostream>


/// \addtogroup gpu_side Implementace vykreslovacího řetězce - vykreslování trojúhelníků
/// @{

/**
 * @brief This function should draw triangles
 *
 * @param gpu gpu 
 * @param nofVertices number of vertices
 */

bool isPointInsideViewFrustum_tr(Vec4 const*const p) {
	// -Aw <= Ai <= +Aw
	for (int i = 0; i < 3; ++i) {
		if (p->data[i] <= -p->data[3])return false;
		if (p->data[i] >= +p->data[3])return false;
	}
	return true;
}

void doPerspectiveDivision_tr(Vec4*const ndc, Vec4 const*const p) {
	for (int a = 0; a < 3; ++a)
		ndc->data[a] = p->data[a] / p->data[3];
	ndc->data[3] = p->data[3];
}

Vec4 computeFragPosition_tr(Vec4 const&p, uint32_t width, uint32_t height) {
	Vec4 res;
	res.data[0] = (p.data[0] * .5f + .5f)*width;
	res.data[1] = (p.data[1] * .5f + .5f)*height;
	res.data[2] = p.data[2];
	res.data[3] = p.data[3];
	return res;
}

void copyVertexAttribute_tr(GPU const*const gpu, GPUAttribute*const att, GPUVertexPullerHead const*const head, uint64_t vertexId) {
	if (!head->enabled)return;
	GPUBuffer const*const buf = gpu_getBuffer(gpu, head->bufferId);
	uint8_t const*ptr = (uint8_t*)buf->data;
	uint32_t const offset = (uint32_t)head->offset;
	uint32_t const stride = (uint32_t)head->stride;
	uint32_t const size = (uint32_t)head->type;
	memcpy(att->data, ptr + offset + vertexId * stride, size);
}

void lobotomized_vertexPuller_tr(GPUInVertex*const inVertex, GPUVertexPuller const*const vao, GPU const*const gpu, uint32_t vertexSHaderInvocation) {
	if (gpu_isBuffer(gpu, vao->indices.bufferId)) {
		GPUBuffer const*const buf = gpu_getBuffer(gpu, vao->indices.bufferId);
		if (vao->indices.type == UINT8) {
			inVertex->gl_VertexID = (uint32_t)((uint8_t*)buf->data)[vertexSHaderInvocation];
		}
		else if (vao->indices.type == UINT16) {
			inVertex->gl_VertexID = (uint32_t)((uint16_t*)buf->data)[vertexSHaderInvocation];
		}
		else {
			inVertex->gl_VertexID = (uint32_t)((uint32_t*)buf->data)[vertexSHaderInvocation];
		}
	}
	else {
		inVertex->gl_VertexID = vertexSHaderInvocation;
	}


	for (int i = 0; i < MAX_ATTRIBUTES; i++) 
	{
		copyVertexAttribute_tr(gpu, inVertex->attributes + i, vao->heads + i, inVertex->gl_VertexID);
	}
}

void lobotomized_pointRasterization_tr(GPUInFragment*const inFragment, Vec4 ndc, GPU*const gpu, GPUOutVertex const*const outVertex) {
	Vec4 coord = computeFragPosition_tr(ndc, gpu->framebuffer.width, gpu->framebuffer.height);
	inFragment->gl_FragCoord = coord;
	memcpy(inFragment->attributes[0].data, outVertex->attributes[0].data, sizeof(Vec4));
}

void lobotomized_perFragmentOperation_tr(GPUOutFragment const*const outFragment, GPU*const gpu, Vec4 ndc) {
	Vec4 coord = computeFragPosition_tr(ndc, gpu->framebuffer.width, gpu->framebuffer.height);
	GPUFramebuffer*const frame = &gpu->framebuffer;
	if (coord.data[0] < 0 || coord.data[0] >= frame->width)return;
	if (coord.data[1] < 0 || coord.data[1] >= frame->height)return;
	if (isnan(coord.data[0]))return;
	if (isnan(coord.data[1]))return;
	uint32_t const pixCoord = frame->width*(int)coord.data[1] + (int)coord.data[0];

	frame->color[pixCoord] = outFragment->gl_FragColor;
}

/**
 * @brief This function draws points
 *
 * @param gpu gpu
 * @param nofVertices number of vertices
 */


void gpu_drawTriangles(GPU *const gpu, uint32_t nofVertices)
{

  /// \todo Naimplementujte vykreslování trojúhelníků.
  /// nofVertices - počet vrcholů
  /// gpu - data na grafické kartě
  /// Vašim úkolem je naimplementovat chování grafické karty.
  /// Úkol je složen:
  /// 1. z implementace Vertex Pulleru
  /// 2. zavolání vertex shaderu pro každý vrchol
  /// 3. rasterizace
  /// 4. zavolání fragment shaderu pro každý fragment
  /// 5. zavolání per fragment operací nad fragmenty (depth test, zápis barvy a hloubky)
  /// Více v připojeném videu.
  (void)gpu;
  (void)nofVertices;
  GPUProgram      const* prg = gpu_getActiveProgram(gpu);
  GPUVertexPuller const* vao = gpu_getActivePuller(gpu);

  GPUVertexShaderData   vd;
  GPUFragmentShaderData fd;

  vd.uniforms = &prg->uniforms;

  for (uint32_t v = 0; v < nofVertices; ++v) {

	  lobotomized_vertexPuller_tr(&vd.inVertex, vao, gpu, v);

	  prg->vertexShader(&vd);

	  Vec4 pos;
	  copy_Vec4(&pos, &vd.outVertex.gl_Position);
	  if (!isPointInsideViewFrustum_tr(&pos))continue;

	  Vec4 ndc;
	  doPerspectiveDivision_tr(&ndc, &pos);

	  lobotomized_pointRasterization_tr(&fd.inFragment, ndc, gpu, &vd.outVertex);

	  prg->fragmentShader(&fd);

	  lobotomized_perFragmentOperation_tr(&fd.outFragment, gpu, ndc);
  }

}


/// @}
