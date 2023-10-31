import os
from conans import ConanFile, CMake

class ConanProduct(ConanFile):
    name = "fep3_encoded_video_plugin"
    version = "0.1.0"
    description = "FEP3 sencoded video streams plugin"
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake", "txt", "CMakeDeps"
    default_user = "local"
    default_channel = "testing"
    options = []
    default_options = {
        "fast-dds:shared": False
    }

    scm = {
        "type": "git",
        "url": "auto",
        "revision": "auto"
    }

    def configure(self):
        import json
        testing_internal = json.loads(self.env.get("enable_testing", "false"))
        self.enable_testing  =  testing_internal if testing_internal else json.loads(os.environ.get("enable_testing", "false"))
        return super().configure()

    def build_requirements(self):
        self.tool_requires("cmake/3.25.0")

    def requirements(self):
        self.requires("fep_sdk_participant/3.1.0", private=True)
        self.requires("fep_fast_dds_plugin/0.1.0@local/testing", private=True)
        #self.requires("ffmpeg/5.1", private=True)
        self.requires("avcpp/2.1.0", private=True)
        self.requires("freetype/2.13.0", override=True)
        self.requires("sdl_ttf/2.20.1", private=True)
        # opengl for player
        # self.requires("sdl/2.26.1", private=True)
        # usb camera (or use opencv?)
        # self.requires("libuvc/0.0.7", private=True)
        

        if self.enable_testing:
            self.requires("gtest/1.10.0", private=True)

    def build(self):
        cmake = CMake(self)
        cmake.definitions["CMAKE_BUILD_TYPE"] = str(self.settings.build_type).upper()
        cmake.configure(defs={"PRODUCT_VERSION": self.version, 
                              "ENABLE_TESTS": str(self.enable_testing),
                            }
                        )
        cmake.build()
        if self.enable_testing:
             cmake.test()
        cmake.install()
