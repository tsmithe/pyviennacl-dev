#include "viennacl.h"
#include "vector.h"

#include <viennacl/ocl/context.hpp>
#include <viennacl/ocl/device.hpp>
#include <viennacl/ocl/platform.hpp>

/*
  
  Notes on platform support; esp. OpenCL
  ======================================
  
  PyOpenCL Integration
  --------------------
  
  * Want to take advantage of PyOpenCL's int_ptr to access ViennaCL
    objects in other (Py)OpenCL code and vice versa.

  * need to get the underlying OpenCL pointers out of ViennaCL --
    and be able to pass them back in


  Multi-platform support
  ----------------------

  * useful to specify the backend details on creation of a ViennaCL
    object (eg matrix)
    + what about copying between back-ends?

  * how to define 'back-end'? := context?

  * throw an exception if attempting to do an operation across
    back-ends


  Scheduler integration and linking
  ---------------------------------

  * users able to define a custom Node class
    + useful for prototyping algorithm implementations!
    + so: useful to expose an API similar to the underlying
          ViennaCL structure

  * class info determines dtypes, arguments and return type
    
  * class provides kernel (source, binary, whatever supported by
    back-end)
    + device-specific kernel support?

  * PyViennaCL registers custom Node with ViennaCL scheduler,
    assigning the Node an ID

  * ViennaCL then has a mapping of (ID, custom operation) pairs

  * when the scheduler is called with the relevant ID, argument types
    are checked and operation is scheduled for dispatch
    + how is the return type created?

  * posible to harness the generator usefully?

  */

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

PYVCL_SUBMODULE(opencl_support)
{

  bp::object opencl_support_submodule(bp::handle<>(bp::borrowed(PyImport_AddModule("_viennacl.opencl_support"))));
  bp::scope().attr("opencl_support") = opencl_support_submodule;
  bp::scope opencl_support_scope = opencl_support_submodule;

  bp::class_<vcl::ocl::platform>("platform", bp::no_init)
    .add_property("info", &vcl::ocl::platform::info)
    .add_property("devices", get_platform_devices)
  ;
  bp::to_python_converter<std::vector<vcl::ocl::platform>,
                          vector_to_list_converter<vcl::ocl::platform> >();

  bp::def("get_platforms", vcl::ocl::get_platforms);

  bp::class_<vcl::ocl::device>("device")
    .add_property("name", &vcl::ocl::device::name)
    .add_property("vendor", &vcl::ocl::device::vendor)
    .add_property("version", &vcl::ocl::device::version)
    .add_property("driver_version", &vcl::ocl::device::driver_version)
    .add_property("info", get_device_info)
    .add_property("full_info", get_device_full_info)
    .add_property("extensions", &vcl::ocl::device::extensions)
    .add_property("double_support", &vcl::ocl::device::double_support)
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
  bp::class_<vcl::ocl::context>("context")
  .def("init_new_context", init_new_context)
  .def("current_device", &vcl::ocl::context::current_device, 
       bp::return_value_policy<bp::copy_const_reference>())
  .def("devices", &vcl::ocl::context::devices,
       bp::return_value_policy<bp::copy_const_reference>())
  .def("add_device", context_add_device)
  .def("switch_active_device", context_switch_device)
  .add_property("platform_index", context_get_platform_index, context_set_platform_index)
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

}
