from conans import ConanFile, CMake, tools
import os


class BenchmarkPlaygroundConan(ConanFile):
    name = "scheduler"
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"

    requires = (
        "benchmark/1.5.0",
        "fmt/6.1.2",
        "glog/0.4.0",
    )

    def source(self):
        pass

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        pass

    def package_info(self):
        pass