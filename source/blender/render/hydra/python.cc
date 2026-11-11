/* SPDX-FileCopyrightText: 2011-2022 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include "final_engine.hh"
#include "preview_engine.hh"
#include "viewport_engine.hh"

#include <Python.h>

#include <exception>
#include <string>

#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec4f.h>

#include "RE_engine.h"

#include "../generic/py_capi_utils.hh"
#include "bpy_rna.hh"

#include "BKE_context.hh"
#include "BKE_report.hh"

#include "RNA_prototypes.hh"

#include "hydra/image.hh"

namespace blender {

namespace render::hydra {

template<typename T> T *pyrna_to_pointer(PyObject *pyobject, const StructRNA *rnatype)
{
  const PointerRNA *ptr = pyrna_struct_as_ptr_or_null(pyobject, rnatype);
  return (ptr) ? static_cast<T *>(ptr->data) : nullptr;
}

static PyObject *engine_create_func(PyObject * /*self*/, PyObject *args)
{
  PyObject *pyengine;
  char *engine_type, *render_delegate_id;
  if (!PyArg_ParseTuple(args,
                        "O" /* `engine` */
                        "s" /* `engine_type` */
                        "s" /* `render_delegate_id` */
                        ":engine_create",
                        &pyengine,
                        &engine_type,
                        &render_delegate_id))
  {
    return nullptr;
  }

  RenderEngine *bl_engine = pyrna_to_pointer<RenderEngine>(pyengine, RNA_RenderEngine);

  CLOG_DEBUG(LOG_HYDRA_RENDER, "Engine %s", engine_type);
  Engine *engine = nullptr;
  std::string error_message;
  try {
    if (STREQ(engine_type, "VIEWPORT")) {
      engine = new ViewportEngine(bl_engine, render_delegate_id);
    }
    else if (STREQ(engine_type, "PREVIEW")) {
      engine = new PreviewEngine(bl_engine, render_delegate_id);
    }
    else {
      engine = new FinalEngine(bl_engine, render_delegate_id);
    }
  }
  /* USD/Tf errors and Arras `DefinitionLoadError` (e.g. from
   * `SessionDefinition::load("hd_single")` when `ARRAS_SESSION_PATH` is wrong)
   * do not necessarily derive from `std::runtime_error`. Catching only that
   * type let them escape and terminate the process, so catch everything. */
  catch (const std::exception &e) {
    error_message = e.what();
  }
  catch (...) {
    error_message = "Unknown exception while creating the Hydra render delegate";
  }

  if (!error_message.empty() || !engine) {
    if (error_message.empty()) {
      error_message = "Cannot create render delegate: " + std::string(render_delegate_id);
    }
    CLOG_ERROR(LOG_HYDRA_RENDER, "%s", error_message.c_str());

    /* Surface the failure in Blender's UI instead of returning a null pointer
     * that `_bpy_types.py` silently ignored. `RE_engine_set_error_message`
     * shows it on the render result; the Info-editor report is emitted once by
     * `HydraRenderEngine._report_engine_error()` on the Python side, so do not
     * also call `RE_engine_report` here (that would duplicate the message). */
    if (bl_engine) {
      RE_engine_set_error_message(bl_engine, error_message.c_str());
    }
    PyErr_SetString(PyExc_RuntimeError, error_message.c_str());
    return nullptr;
  }

  CLOG_DEBUG(LOG_HYDRA_RENDER, "Engine %p", engine);
  return PyLong_FromVoidPtr(engine);
}

static PyObject *engine_free_func(PyObject * /*self*/, PyObject *args)
{
  PyObject *pyengine;
  if (!PyArg_ParseTuple(args,
                        "O" /* `engine` */
                        ":engine_free",
                        &pyengine))
  {
    return nullptr;
  }

  Engine *engine = static_cast<Engine *>(PyLong_AsVoidPtr(pyengine));
  CLOG_DEBUG(LOG_HYDRA_RENDER, "Engine %p", engine);
  delete engine;

  Py_RETURN_NONE;
}

static PyObject *engine_update_func(PyObject * /*self*/, PyObject *args)
{
  PyObject *pyengine, *pydepsgraph, *pycontext;
  if (!PyArg_ParseTuple(args,
                        "O" /* `engine` */
                        "O" /* `depsgraph` */
                        "O" /* `context` */
                        ":engine_update",
                        &pyengine,
                        &pydepsgraph,
                        &pycontext))
  {
    return nullptr;
  }

  Engine *engine = static_cast<Engine *>(PyLong_AsVoidPtr(pyengine));
  if (!engine) {
    PyErr_SetString(PyExc_RuntimeError, "Hydra render engine is not available");
    return nullptr;
  }
  Depsgraph *depsgraph = pyrna_to_pointer<Depsgraph>(pydepsgraph, RNA_Depsgraph);
  bContext *context = pyrna_to_pointer<bContext>(pycontext, RNA_Context);

  CLOG_DEBUG(LOG_HYDRA_RENDER, "Engine %p", engine);
  try {
    engine->sync(depsgraph, context);
  }
  catch (const std::exception &e) {
    CLOG_ERROR(LOG_HYDRA_RENDER, "%s", e.what());
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return nullptr;
  }
  catch (...) {
    PyErr_SetString(PyExc_RuntimeError, "Unknown exception while syncing the Hydra scene");
    return nullptr;
  }

  Py_RETURN_NONE;
}

static PyObject *engine_render_func(PyObject * /*self*/, PyObject *args)
{
  PyObject *pyengine;
  if (!PyArg_ParseTuple(args,
                        "O" /* `engine` */
                        ":engine_render",
                        &pyengine))
  {
    return nullptr;
  }

  Engine *engine = static_cast<Engine *>(PyLong_AsVoidPtr(pyengine));
  if (!engine) {
    PyErr_SetString(PyExc_RuntimeError, "Hydra render engine is not available");
    return nullptr;
  }

  CLOG_DEBUG(LOG_HYDRA_RENDER, "Engine %p", engine);

  /* Allow Blender to execute other Python scripts. An exception crossing this
   * boundary used to terminate the process; convert it into a Python error so
   * the failure is reported instead of swallowed or fatal. */
  std::string error_message;
  Py_BEGIN_ALLOW_THREADS;
  try {
    engine->render();
  }
  catch (const std::exception &e) {
    error_message = e.what();
  }
  catch (...) {
    error_message = "Unknown exception while rendering with Hydra";
  }
  Py_END_ALLOW_THREADS;

  if (!error_message.empty()) {
    CLOG_ERROR(LOG_HYDRA_RENDER, "%s", error_message.c_str());
    PyErr_SetString(PyExc_RuntimeError, error_message.c_str());
    return nullptr;
  }

  Py_RETURN_NONE;
}

static PyObject *engine_view_draw_func(PyObject * /*self*/, PyObject *args)
{
  PyObject *pyengine, *pycontext;
  if (!PyArg_ParseTuple(args,
                        "O" /* `engine` */
                        "O" /* `context` */
                        ":engine_view_draw",
                        &pyengine,
                        &pycontext))
  {
    return nullptr;
  }

  ViewportEngine *engine = static_cast<ViewportEngine *>(PyLong_AsVoidPtr(pyengine));
  if (!engine) {
    PyErr_SetString(PyExc_RuntimeError, "Hydra render engine is not available");
    return nullptr;
  }
  bContext *context = pyrna_to_pointer<bContext>(pycontext, RNA_Context);

  CLOG_DEBUG(LOG_HYDRA_RENDER, "Engine %p", engine);

  /* Allow Blender to execute other Python scripts. */
  std::string error_message;
  Py_BEGIN_ALLOW_THREADS;
  try {
    engine->render(context);
  }
  catch (const std::exception &e) {
    error_message = e.what();
  }
  catch (...) {
    error_message = "Unknown exception while drawing the Hydra viewport";
  }
  Py_END_ALLOW_THREADS;

  if (!error_message.empty()) {
    CLOG_ERROR(LOG_HYDRA_RENDER, "%s", error_message.c_str());
    PyErr_SetString(PyExc_RuntimeError, error_message.c_str());
    return nullptr;
  }

  Py_RETURN_NONE;
}

static pxr::VtValue get_setting_val(PyObject *pyval)
{
  pxr::VtValue val;
  if (PyBool_Check(pyval)) {
    val = Py_IsTrue(pyval);
  }
  else if (PyLong_Check(pyval)) {
    val = PyLong_AsLong(pyval);
  }
  else if (PyFloat_Check(pyval)) {
    val = PyFloat_AsDouble(pyval);
  }
  else if (PyUnicode_Check(pyval)) {
    val = std::string(PyUnicode_AsUTF8(pyval));
  }
  else if (PyTuple_Check(pyval) || PyList_Check(pyval)) {
    PyObject *sequence = PySequence_Fast(pyval, "Expected a numeric vector");
    if (!sequence) {
      return val;
    }

    const Py_ssize_t size = PySequence_Fast_GET_SIZE(sequence);
    if (size < 2 || size > 4) {
      PyErr_SetString(PyExc_ValueError, "Render setting vectors must have 2, 3, or 4 values");
      Py_DECREF(sequence);
      return val;
    }

    double values[4] = {};
    PyObject **items = PySequence_Fast_ITEMS(sequence);
    for (Py_ssize_t i = 0; i < size; i++) {
      values[i] = PyFloat_AsDouble(items[i]);
      if (PyErr_Occurred()) {
        Py_DECREF(sequence);
        return val;
      }
    }
    Py_DECREF(sequence);

    if (size == 2) {
      val = pxr::GfVec2f(float(values[0]), float(values[1]));
    }
    else if (size == 3) {
      val = pxr::GfVec3f(float(values[0]), float(values[1]), float(values[2]));
    }
    else {
      val = pxr::GfVec4f(float(values[0]), float(values[1]), float(values[2]), float(values[3]));
    }
  }
  return val;
}

static PyObject *engine_set_render_setting_func(PyObject * /*self*/, PyObject *args)
{
  PyObject *pyengine, *pyval;
  char *key;
  if (!PyArg_ParseTuple(args,
                        "O" /* `engine` */
                        "s" /* `key` */
                        "O" /* `value` */
                        ":engine_set_render_setting",
                        &pyengine,
                        &key,
                        &pyval))
  {
    return nullptr;
  }

  Engine *engine = static_cast<Engine *>(PyLong_AsVoidPtr(pyengine));
  if (!engine) {
    PyErr_SetString(PyExc_RuntimeError, "Hydra render engine is not available");
    return nullptr;
  }

  CLOG_DEBUG(LOG_HYDRA_RENDER, "Engine %p: %s", engine, key);
  const pxr::VtValue value = get_setting_val(pyval);
  if (value.IsEmpty()) {
    if (!PyErr_Occurred()) {
      PyErr_SetString(PyExc_TypeError,
                      "Render settings must be booleans, integers, floats, strings, or numeric "
                      "vectors");
    }
    return nullptr;
  }
  engine->set_render_setting(key, value);

  Py_RETURN_NONE;
}

static PyObject *cache_or_get_image_file_func(PyObject * /*self*/, PyObject *args)
{
  PyObject *pycontext, *pyimage;
  if (!PyArg_ParseTuple(args,
                        "O" /* `context` */
                        "O" /* `image` */
                        ":cache_or_get_image_file",
                        &pycontext,
                        &pyimage))
  {
    return nullptr;
  }

  bContext *context = static_cast<bContext *>(PyLong_AsVoidPtr(pycontext));
  Image *image = static_cast<Image *>(PyLong_AsVoidPtr(pyimage));

  std::string image_path = io::hydra::cache_or_get_image_file(
      CTX_data_main(context), CTX_data_scene(context), image, nullptr);
  return PyC_UnicodeFromStdStr(image_path);
}

static PyMethodDef methods[] = {
    {"engine_create", engine_create_func, METH_VARARGS, ""},
    {"engine_free", engine_free_func, METH_VARARGS, ""},
    {"engine_update", engine_update_func, METH_VARARGS, ""},
    {"engine_render", engine_render_func, METH_VARARGS, ""},
    {"engine_view_draw", engine_view_draw_func, METH_VARARGS, ""},
    {"engine_set_render_setting", engine_set_render_setting_func, METH_VARARGS, ""},

    {"cache_or_get_image_file", cache_or_get_image_file_func, METH_VARARGS, ""},

    {nullptr, nullptr, 0, nullptr},
};

static PyModuleDef module = {
    PyModuleDef_HEAD_INIT,
    "_bpy_hydra",
    "Hydra render API",
    -1,
    methods,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
};

}  // namespace render::hydra

PyObject *BPyInit_hydra();

PyObject *BPyInit_hydra()
{
  PyObject *mod = PyModule_Create(&render::hydra::module);
  return mod;
}

}  // namespace blender
