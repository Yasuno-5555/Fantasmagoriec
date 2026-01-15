#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "view/api.hpp"
#include "core/contexts_internal.hpp"

namespace py = pybind11;
using namespace fanta;
using namespace fanta::ui;

// Helper to check if backend is running (Exposing internal state to Py)
bool is_running() {
    auto& ctx = internal::GetEngineContext();
    return ctx.world.backend && ctx.world.backend->is_running();
}

// Universal Mixin Bindings (The Power of Phase 5.5)
template <typename T, typename PyClass>
void BindLayoutConfig(PyClass& cls) {
    cls.def("width", &T::width, py::return_value_policy::reference)
       .def("height", &T::height, py::return_value_policy::reference)
       .def("size", py::overload_cast<float, float>(&T::size), py::return_value_policy::reference)
       .def("grow", &T::grow, py::return_value_policy::reference)
       .def("shrink", &T::shrink, py::return_value_policy::reference)
       .def("padding", py::overload_cast<float>(&T::padding), py::return_value_policy::reference)
       .def("margin", py::overload_cast<float>(&T::margin), py::return_value_policy::reference)
       .def("align", &T::align, py::return_value_policy::reference);
}

template <typename T, typename PyClass>
void BindStyleConfig(PyClass& cls) {
    cls.def("bg", &T::bg, py::return_value_policy::reference)
       .def("color", &T::color, py::return_value_policy::reference)
       .def("radius", &T::radius, py::return_value_policy::reference)
       .def("shadow", &T::shadow, py::return_value_policy::reference)
       .def("elevation", &T::elevation, py::return_value_policy::reference)
       .def("blur", &T::blur, py::return_value_policy::reference)
       .def("wobble", &T::wobble, py::return_value_policy::reference)
       .def("size", py::overload_cast<float>(&T::size), py::return_value_policy::reference) // Font size
       .def("red", &T::red, py::return_value_policy::reference)
       .def("dark", &T::dark, py::return_value_policy::reference);
}

PYBIND11_MODULE(fanta, m) {
    m.doc() = "Fantasmagorie V5 Python Binding";

    // --- Core Types ---
    py::class_<internal::ColorF>(m, "Color")
        .def(py::init<float, float, float, float>(), py::arg("r"), py::arg("g"), py::arg("b"), py::arg("a")=1.0f);

    py::enum_<Align>(m, "Align")
        .value("Start", Align::Start)
        .value("Center", Align::Center)
        .value("End", Align::End)
        .value("Stretch", Align::Stretch);

    // --- Engine Control ---
    m.def("init", &Init, "Initialize the engine", py::arg("width"), py::arg("height"), py::arg("title"));
    m.def("running", &is_running, "Check if the engine is running");
    m.def("begin_frame", &BeginFrame, "Start a new frame");
    m.def("end_frame", []() {
        // EndFrame in C++ usually handles internal renders, but we need DrawList
        // So we expose a simpler flow or render directly.
        // For PyFanta, we'll use a combined helper.
    });

    // --- Engine Control ---
    m.def("init", &Init, "Initialize the engine", py::arg("width"), py::arg("height"), py::arg("title"));
    m.def("running", &is_running, "Check if the engine is running");
    m.def("begin_frame", &BeginFrame, "Start a new frame");
    
    m.def("render_ui", [](fanta::ui::ViewHeader* root, float w, float h) {
        auto& ctx = internal::GetEngineContext();
        internal::DrawList dl;
        fanta::ui::RenderUI(root, w, h, dl);
        if (ctx.world.backend) {
            ctx.world.backend->render(dl);
            ctx.world.backend->end_frame();
        }
    }, "Render the UI tree");

    m.def("get_width", &GetScreenWidth);
    m.def("get_height", &GetScreenHeight);

    // --- Configs / Builders ---
    
    // Box
    auto box = py::class_<BoxConfig>(m, "BoxConfig");
    BindLayoutConfig<BoxConfig>(box);
    BindStyleConfig<BoxConfig>(box);
    m.def("Box", &Box, py::return_value_policy::reference);
    m.def("Row", &Row, py::return_value_policy::reference);
    m.def("Column", &Column, py::return_value_policy::reference);
    m.def("End", &End);

    // Text
    auto text = py::class_<TextConfig>(m, "TextConfig");
    BindLayoutConfig<TextConfig>(text);
    BindStyleConfig<TextConfig>(text);
    m.def("Text", &Text, py::return_value_policy::reference);

    // Button
    auto btn = py::class_<ButtonConfig>(m, "ButtonConfig");
    BindLayoutConfig<ButtonConfig>(btn);
    BindStyleConfig<ButtonConfig>(btn);
    btn.def("__bool__", &ButtonConfig::operator bool); // if Button("Click"):
    m.def("Button", &Button, py::return_value_policy::reference);

    // Scroll
    auto scroll = py::class_<ScrollConfig>(m, "ScrollConfig");
    BindLayoutConfig<ScrollConfig>(scroll);
    BindStyleConfig<ScrollConfig>(scroll);
    m.def("Scroll", &Scroll, py::arg("vertical")=true, py::return_value_policy::reference);

    // TextInput
    // For now, simpler TextInput that uses a persistent string in Py is harder.
    // We'll bind a minimal version if needed, or stick to Statics for now.
    
    // Splitter
    auto split = py::class_<SplitterConfig>(m, "SplitterConfig");
    BindLayoutConfig<SplitterConfig>(split);
    BindStyleConfig<SplitterConfig>(split);
    // Note: Ratio binding needs care with pointers.
}
