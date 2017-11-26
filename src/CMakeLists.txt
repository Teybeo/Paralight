set(SOURCE_FILES ${SOURCE_FILES}
        app/main.cpp
        app/App.cpp app/App.h
        app/Window.cpp app/Window.h app/Chronometer.h)

set(SOURCE_FILES ${SOURCE_FILES}
        core/Scene.cpp core/Scene.h
        core/CameraControls.cpp core/CameraControls.h
        core/Ray.h
        core/Random.h core/Random.cpp
        core/Options.cpp core/Options.h
        core/Texture.cpp core/Texture.h
        objects/BoundingBox.h objects/BoundingBox.cpp
        objects/SurfaceData.cpp objects/SurfaceData.h
        core/BVHCommons.h
        core/BVH.cpp core/BVH.h
        core/BVH2.cpp core/BVH2.h)

set(SOURCE_FILES ${SOURCE_FILES}
        renderers/BaseRenderer.cpp renderers/BaseRenderer.h
        renderers/CppRenderer.cpp renderers/CppRenderer.h
        renderers/OpenCLRenderer.cpp renderers/OpenCLRenderer.h)

set(SOURCE_FILES ${SOURCE_FILES}
        objects/Object3D.cpp objects/Object3D.h
        objects/Sphere.cpp objects/Sphere.h
        objects/Plane.cpp objects/Plane.h
        objects/TriMesh.cpp objects/TriMesh.h
        objects/Triangle.cpp objects/Triangle.h
        objects/Intersectable.h objects/Intersectable.cpp)

set(SOURCE_FILES ${SOURCE_FILES}
        core/Material.cpp core/Material.h
        material/BrdfStack.cpp material/BrdfStack.h
        material/Brdf.h
        )

set(SOURCE_FILES ${SOURCE_FILES}
    hdrloader/hdrloader.cpp hdrloader/hdrloader.h)

set(SOURCE_FILES ${SOURCE_FILES}
        math/Matrix.cpp math/Matrix.h
        math/Vec3.cpp math/Vec3.h
        math/TrigoLut.h math/TrigoLut.cpp)

set(SOURCE_FILES ${SOURCE_FILES}
        opencl/Program.cpp opencl/Program.h
        opencl/ProgramBuilder.cpp opencl/ProgramBuilder.h
        opencl/OpenCLPlatformList.cpp opencl/OpenCLPlatformList.h
        opencl/SceneAdapter.cpp opencl/SceneAdapter.h)

set(SOURCE_FILES ${SOURCE_FILES}
        gui/Gui.cpp gui/Gui.h
        gui/imgui/imgui.cpp gui/imgui/imgui.h
        gui/imgui/imgui_draw.cpp
        gui/imgui/imgui_impl_sdl.cpp gui/imgui/imgui_impl_sdl.h
        gui/imgui/imgui_internal.h
        gui/imgui/imgui_draw.cpp
        gui/imgui/stb_rect_pack.h
        gui/imgui/stb_textedit.h
        gui/imgui/stb_truetype.h)

#set(SOURCE_FILES ${SOURCE_FILES}
#        lights_unused/Light.cpp lights_unused/Light.h
#        lights_unused/PointLight.cpp lights_unused/PointLight.h)

message("---- Source files ----")
foreach(src ${SOURCE_FILES})
    message("${src}")
endforeach()

message("---- Include directories ----")
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
    message("${dir}")
endforeach()

message("---- Libraries ----")
foreach(lib ${LIBRARIES})
    message("${lib}")
endforeach()

add_executable("Paralight"        ${SOURCE_FILES})
target_link_libraries ("Paralight" PRIVATE   ${LIBRARIES})
target_compile_options("Paralight" PRIVATE ${COMMON_FLAGS})
target_compile_definitions("Paralight" PRIVATE ${DEFINITIONS})
