from SCons.Script import DefaultEnvironment # type: ignore

env = DefaultEnvironment()

# Automatically add BB_DEBUG if this is a debug build
if env.get("build_type", "") == "debug":
    env.Append(CPPDEFINES=["BB_DEBUG"])
