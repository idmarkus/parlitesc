#pragma once

#include <string>

namespace PLSC::GL
{
    static const std::string vertCircle = "#version 330 core\n"
                                          "precision highp float;\n"
                                          "layout (location = 0) in vec2 vPosition;\n"
                                          "layout (location = 1) in vec2 vOffset;\n"
                                          "out vec2 fOffset;\n"
                                          "out float fRadius;\n"
                                          "uniform vec2 worldSize;\n"
                                          "uniform vec2 screenSize;\n"
                                          "uniform float radius;\n"
                                          "void main() {\n"
                                          "    vec2 rel = screenSize / worldSize;\n"
                                          "    float scalar = (rel.x < rel.y) ? rel.x : rel.y;\n"
                                          "    vec2 nOfs = vOffset - (worldSize * 0.5);\n"
                                          "    nOfs *= scalar;\n"
                                          "    nOfs /= screenSize * 0.5;\n"
                                          "    nOfs = vec2(nOfs.x, nOfs.y*(-1.0));\n"
                                          "    fRadius = radius * scalar;\n"
                                          "    vec2 scale = (fRadius*2.0) / screenSize;\n"
                                          "    vec2 nPos = scale * vPosition;\n"
                                          "    nPos += nOfs;\n"
                                          "    fOffset = ((nOfs + 1.0) / 2.0) * screenSize;\n"
                                          "    gl_Position = vec4(nPos, 0.0, 1.0);\n"
                                          "}";
    static const std::string fragCircle = "#version 330 core\n"
                                          "precision highp float;\n"
                                          "out vec4 FragColor;\n"
                                          "in vec2 fOffset;\n"
                                          "in float fRadius;\n"
                                          "void main() {\n"
                                          "    float d = length(gl_FragCoord.xy - fOffset) / fRadius;\n"
                                          "    if (d > 1.0) discard;\n"
                                          "    float v = 1.0 - pow(d, 16);\n"
                                          "    FragColor = vec4(v);\n"
                                          "}";
    static const std::string vertRect   = "#version 330 core\n"
                                          "precision highp float;\n"
                                          "layout (location = 0) in vec2 worldPos;\n"
                                          "uniform vec2 worldSize;\n"
                                          "uniform vec2 screenSize;\n"
                                          "void main() {\n"
                                          "    vec2 nP = worldPos - (worldSize * 0.5);\n"
                                          "    vec2 rel = screenSize / worldSize;\n"
                                          "    nP *= (rel.x < rel.y) ? rel.x : rel.y;\n"
                                          "    nP /= screenSize * 0.5;\n"
                                          "    gl_Position = vec4(nP.x, nP.y * -1.0, 0.0, 1.0);\n"
                                          "}";
    static const std::string fragRect   = "#version 330 core\n"
                                          "precision highp float;\n"
                                          "out vec4 FragColor;\n"
                                          "void main() {\n"
                                          "    FragColor = vec4(0.5,0.2,0.1,1.0);\n"
                                          "}";
} // namespace PLSC::GL