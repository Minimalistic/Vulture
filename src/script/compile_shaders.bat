@ECHO OFF
SET SHADER_SOURCE_DIR=%1
SET SHADER_COMPILER=%2
SET GLSL_VERSION=%3
SET BUF_COUNT=%4
SET INST_COUNT=%5
SET SHADER_BINARY_DIR=shaders

IF NOT EXIST "%SHADER_BINARY_DIR%\" MKDIR "%SHADER_BINARY_DIR%"

SET BUILD_DIR=%cd%
CD %SHADER_SOURCE_DIR%
FOR %%F IN (*.comp) DO (
    COPY "%%F" "%%F.tmp"
    ECHO #version %GLSL_VERSION% core > "%%F"
    ECHO #define BUFFER_COUNT %BUF_COUNT% >> "%%F"
    TYPE "%%F.tmp" >> "%%F"
    %SHADER_COMPILER% -V "%%F" -o "%%F.spv"
    MOVE "%%F.tmp" "%%F"
)
FOR %%F IN (*.vert) DO (
    COPY "%%F" "%%F.tmp"
    ECHO #version %GLSL_VERSION% core > "%%F"
    ECHO #define INSTANCE_COUNT %INST_COUNT% >> "%%F"
    TYPE "%%F.tmp" >> "%%F"
    %SHADER_COMPILER% -V "%%F" -o "%%F.spv"
    MOVE "%%F.tmp" "%%F"
)
FOR %%F IN (*.frag) DO (
    COPY "%%F" "%%F.tmp"
    ECHO #version %GLSL_VERSION% core > "%%F"
    TYPE "%%F.tmp" >> "%%F"
    %SHADER_COMPILER% -V "%%F" -o "%%F.spv"
    MOVE "%%F.tmp" "%%F"
)
MOVE *.spv "%BUILD_DIR%\%SHADER_BINARY_DIR%" >NUL 2>&1
