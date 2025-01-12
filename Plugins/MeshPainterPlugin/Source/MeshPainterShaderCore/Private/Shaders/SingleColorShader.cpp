#include "Shaders/SingleColorShader.h"

IMPLEMENT_GLOBAL_SHADER(FSingleColorShaderVS, "/Plugin/MeshPainterPlugin/SingleColorShader.usf", "ColorFillVS", SF_Vertex);
IMPLEMENT_GLOBAL_SHADER(FSingleColorShaderPS, "/Plugin/MeshPainterPlugin/SingleColorShader.usf", "ColorFillPS", SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FSingleColorShaderCS, "/Plugin/MeshPainterPlugin/SingleColorShader.usf", "ColorFillCS", SF_Compute);