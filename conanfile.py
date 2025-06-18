from conan import ConanFile

class Application(ConanFile):
    settings = "os", "compiler", "arch", "build_type"
    generators = "PkgConfigDeps", "CMakeDeps"

    requires = (
        "mlpack/4.4.0@aurora",
    )
