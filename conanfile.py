import os
from conans import ConanFile, CMake

class ConanProduct(ConanFile):
    name = "fep3_encoded_video_plugin"
    version = "0.1.0"
    description = "FEP3 encoded video streams plugin"
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake", "txt", "CMakeDeps"
    default_user = "local"
    default_channel = "testing"
    options = []
    default_options = {
        "fast-dds:shared": False,
        "qt:qtmultimedia": True,
        "qt:shared": True
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
        self.requires("fep_sdk_participant/3.1.0@fep_oss/testing", private=True)
        self.requires("fep3_fast_dds_plugin/0.1.0@fep_oss/testing", private=True)
        self.requires("qt/6.6.0", private=True)

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
