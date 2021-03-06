#ifdef VIENNACL_WITH_OPENCL

#include "vector.hpp"

#include <viennacl/ocl/backend.hpp>
#include <viennacl/ocl/platform.hpp>
#include <viennacl/ocl/device.hpp>
#include <viennacl/ocl/context.hpp>
#include <viennacl/ocl/command_queue.hpp>

template<class VCLType, class PtrType>
vcl::tools::shared_ptr<VCLType>
vcl_object_from_int_ptr(vcl::vcl_size_t int_ptr) {
  VCLType *p = new VCLType((PtrType)int_ptr);
  return vcl::tools::shared_ptr<VCLType>(p);
}

vcl::tools::shared_ptr<vcl::ocl::context>
vcl_context_from_int_ptr(vcl::vcl_size_t int_ptr) {
  vcl::ocl::context *c = new vcl::ocl::context();
  c->init((cl_context)int_ptr);
  return vcl::tools::shared_ptr<vcl::ocl::context>(c);
}

vcl::tools::shared_ptr<vcl::ocl::command_queue>
vcl_command_queue_from_int_ptr(const vcl::ocl::context& ctx, vcl::vcl_size_t int_ptr) {
  vcl::ocl::handle<cl_command_queue> h((cl_command_queue)int_ptr, ctx);
  vcl::ocl::command_queue *q = new vcl::ocl::command_queue(h);
  return vcl::tools::shared_ptr<vcl::ocl::command_queue>(q);
}

vcl::vcl_size_t get_platform_ptr(vcl::ocl::platform& p) {
  return (vcl::vcl_size_t) p.id();
}

vcl::vcl_size_t get_device_ptr(vcl::ocl::device& d) {
  return (vcl::vcl_size_t) d.id();
}

vcl::vcl_size_t get_context_ptr(vcl::ocl::context& c) {
  return (vcl::vcl_size_t) c.handle().get();
}

vcl::vcl_size_t get_command_queue_ptr(vcl::ocl::command_queue& q) {
  return (vcl::vcl_size_t) q.handle().get();
}

std::vector<vcl::ocl::device>
get_platform_devices(vcl::ocl::platform& p) {
  return p.devices();
}

std::string get_device_info(vcl::ocl::device& d) {
  return d.info();
}

std::string get_device_full_info(vcl::ocl::device& d) {
  return d.full_info();
}

const vcl::ocl::command_queue& ctx_get_queue(vcl::ocl::context& ctx,
                                             vcl::vcl_size_t device_int_ptr,
                                             vcl::vcl_size_t id) {
  return ctx.get_queue((cl_device_id)device_int_ptr, id);
}

void ctx_add_new_queue(vcl::ocl::context& ctx, vcl::vcl_size_t device) {
  ctx.add_queue((cl_device_id)device);
}

void ctx_add_existing_queue(vcl::ocl::context& ctx,
                            vcl::vcl_size_t device, vcl::vcl_size_t queue) {
  ctx.add_queue((cl_device_id)device, (cl_command_queue)queue);
}

#endif

PYVCL_SUBMODULE(opencl_support)
{

#ifdef VIENNACL_WITH_OPENCL
  PYTHON_SCOPE_SUBMODULE(opencl_support);

  bp::class_<vcl::ocl::backend<> >("backend")
    //.def("add_context", &vcl::ocl::backend<>::add_context)
    .def("switch_context", &vcl::ocl::backend<>::switch_context)
    ;

  bp::class_<vcl::ocl::platform, vcl::tools::shared_ptr<vcl::ocl::platform> >
    ("platform", bp::no_init)
    .def("__init__", bp::make_constructor(vcl_object_from_int_ptr<vcl::ocl::platform, cl_platform_id>))
    .add_property("info", &vcl::ocl::platform::info)
    .add_property("devices", get_platform_devices)
    .add_property("int_ptr", get_platform_ptr)
    ;

  bp::to_python_converter<std::vector<vcl::ocl::platform>,
                          vector_to_list_converter<vcl::ocl::platform> >();

  bp::def("get_platforms", vcl::ocl::get_platforms);

  bp::class_<vcl::ocl::device, vcl::tools::shared_ptr<vcl::ocl::device> >
    ("device")
    .def("__init__", bp::make_constructor(vcl_object_from_int_ptr<vcl::ocl::device, cl_device_id>))
    .add_property("name", &vcl::ocl::device::name)
    .add_property("vendor", &vcl::ocl::device::vendor)
    .add_property("version", &vcl::ocl::device::version)
    .add_property("driver_version", &vcl::ocl::device::driver_version)
    .add_property("info", get_device_info)
    .add_property("full_info", get_device_full_info)
    .add_property("extensions", &vcl::ocl::device::extensions)
    .add_property("double_support", &vcl::ocl::device::double_support)
    .add_property("int_ptr", get_device_ptr)
    ;

  bp::to_python_converter<std::vector<vcl::ocl::device>,
                          vector_to_list_converter<vcl::ocl::device> >();
  
  DISAMBIGUATE_CLASS_FUNCTION_PTR(vcl::ocl::context, void,
                                  init, init_new_context,
                                  ());
  DISAMBIGUATE_CLASS_FUNCTION_PTR(vcl::ocl::context, void,
                                  add_device, context_add_device,
                                  (viennacl::ocl::device const&));
  DISAMBIGUATE_CLASS_FUNCTION_PTR(vcl::ocl::context, void,
                                  switch_device, context_switch_device,
                                  (viennacl::ocl::device const&));
  DISAMBIGUATE_CLASS_FUNCTION_PTR(vcl::ocl::context, void,
                                  platform_index, context_set_platform_index,
                                  (vcl::vcl_size_t));
  DISAMBIGUATE_CLASS_FUNCTION_PTR(vcl::ocl::context, vcl::vcl_size_t,
                                  platform_index, context_get_platform_index,
                                  () const);
  DISAMBIGUATE_CLASS_FUNCTION_PTR(vcl::ocl::context, void,
                                  switch_queue, ctx_switch_queue,
                                  (const vcl::ocl::command_queue&));
  bp::class_<vcl::ocl::context>("context")
    .def("__init__", bp::make_constructor(vcl_context_from_int_ptr))
    .def(bp::self == vcl::ocl::context())
    .def("init_new_context", init_new_context)
    .add_property("current_device",
                  bp::make_function(&vcl::ocl::context::current_device,
                                    bp::return_value_policy<bp::copy_const_reference>()))
    .add_property("devices",
                  bp::make_function(&vcl::ocl::context::devices,
                                    bp::return_value_policy<bp::copy_const_reference>()))
    .def("add_device", context_add_device)
    .def("switch_active_device", context_switch_device)
    .add_property("current_queue",
                  bp::make_function(&vcl::ocl::context::current_queue,
                                    bp::return_value_policy<bp::copy_const_reference>()))
    .def("get_queue", ctx_get_queue, bp::return_value_policy<bp::copy_const_reference>())
    .def("add_new_queue", ctx_add_new_queue)
    .def("add_existing_queue", ctx_add_existing_queue)
    .def("switch_queue", ctx_switch_queue)
    .add_property("platform_index", context_get_platform_index, context_set_platform_index)
    .add_property("int_ptr", get_context_ptr)
    ;

  bp::class_<vcl::ocl::command_queue>("command_queue", bp::no_init)
    .def("__init__", bp::make_constructor(vcl_command_queue_from_int_ptr))
    .add_property("int_ptr", get_command_queue_ptr)
    ;
    
  bp::def("get_current_context", vcl::ocl::current_context,
          bp::return_value_policy<bp::copy_non_const_reference>());
  bp::def("get_current_device", vcl::ocl::current_device,
          bp::return_value_policy<bp::copy_const_reference>());

  DISAMBIGUATE_FUNCTION_PTR(void,
                            vcl::ocl::setup_context,
                            setup_context_single,
                            (long, vcl::ocl::device const&));
  bp::def("setup_context", setup_context_single);

  bp::def("switch_context", vcl::ocl::switch_context);
#endif

}
