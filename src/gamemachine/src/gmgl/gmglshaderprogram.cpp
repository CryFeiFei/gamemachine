﻿#include "stdafx.h"
#include <GL/glew.h>
#include <stdio.h>
#include "gmglshaderprogram.h"
#include "gmglgraphic_engine.h"
#include <regex>
#include "foundation/gamemachine.h"
#include <linearmath.h>

namespace
{
	GMint32 toTechniqueEntranceId(const GMString& instanceName)
	{
		enum
		{
			// ModelType
			Model2D = (GMint32) GMModelType::Model2D,
			Model3D = (GMint32) GMModelType::Model3D,
			Text = (GMint32) GMModelType::Text,
			CubeMap = (GMint32) GMModelType::CubeMap,
			Particle = (GMint32) GMModelType::Particle,
			Custom = (GMint32) GMModelType::Custom,
			Shadow,

			// Filter (gmgraphicengine.h)
			DefaultFilter = 0,
			InversionFilter = 1,
			SharpenFilter = 2,
			BlurFilter = 3,
			GrayscaleFilter = 4,
			EdgeDetectFilter = 5,
		};

		GM_STATIC_ASSERT(Shadow == 8, "If shadow enum value is changed, you have to modify glsl.");

		static const GMString s_Model2D = L"GM_Model2D";
		static const GMString s_Model3D = L"GM_Model3D";
		static const GMString s_Text = L"GM_Text";
		static const GMString s_CubeMap = L"GM_CubeMap";
		static const GMString s_Particle = L"GM_Particle";
		static const GMString s_Custom = L"GM_Custom";
		static const GMString s_Shadow = L"GM_Shadow";

		if (instanceName == s_Model2D)
		{
			return Model2D;
		}
		else if (instanceName == s_Model3D)
		{
			return Model3D;
		}
		else if (instanceName == s_Text)
		{
			return Text;
		}
		else if (instanceName == s_CubeMap)
		{
			return CubeMap;
		}
		else if (instanceName == s_Particle)
		{
			return Particle;
		}
		else if (instanceName == s_Custom)
		{
			return Custom;
		}
		else if (instanceName == s_Shadow)
		{
			return Shadow;
		}
		else
		{
			for (GMint32 i = 0; i < GMFilterCount; ++i)
			{
				if (GM_VariablesDesc.FilterAttributes.Types[i] == instanceName)
					return i;
			}
		}
		GM_ASSERT(false);
		return Model2D;
	}
}

GLuint GMGLShaderProgram::Data::lastUsedProgram = -1;

GMuint32 GMGLShaderInfo::toGLShaderType(GMShaderType type)
{
	switch (type)
	{
	case GMShaderType::Pixel:
		return GL_FRAGMENT_SHADER;
	case GMShaderType::Vertex:
		return GL_VERTEX_SHADER;
	case GMShaderType::Geometry:
		return GL_GEOMETRY_SHADER;
	default:
		GM_ASSERT(false);
		return GL_NONE;
	}
}

GMShaderType GMGLShaderInfo::fromGLShaderType(GMuint32 type)
{
	switch (type)
	{
	case GL_FRAGMENT_SHADER:
		return GMShaderType::Pixel;
	case GL_VERTEX_SHADER:
		return GMShaderType::Vertex;
	case GL_GEOMETRY_SHADER:
		return GMShaderType::Geometry;
	default:
		GM_ASSERT(false);
	}
	return GMShaderType::Unknown;
}

GMGLShaderProgram::GMGLShaderProgram(const IRenderContext* context)
{
	D(d);
	d->context = context;
}

GMGLShaderProgram::~GMGLShaderProgram()
{
	D(d);
	glDeleteProgram(d->shaderProgram);
}

void GMGLShaderProgram::useProgram()
{
	D(d);
	if (d->lastUsedProgram == d->shaderProgram)
		return;
	
	glUseProgram(d->shaderProgram);
	d->lastUsedProgram = d->shaderProgram;
}

void GMGLShaderProgram::attachShader(const GMGLShaderInfo& shaderCfgs)
{
	D(d);
	d->shaderInfos.push_back(shaderCfgs);
}

GMint32 GMGLShaderProgram::getIndex(const GMString& name)
{
	return glGetUniformLocation(getProgram(), name.toStdString().c_str());
}

void GMGLShaderProgram::setMatrix4(GMint32 index, const GMMat4& value)
{
	GM_ASSERT(verify());
	glUniformMatrix4fv(index, 1, GL_FALSE, ValuePointer(value));
}

void GMGLShaderProgram::setVec4(GMint32 index, const GMFloat4& value)
{
	GM_ASSERT(verify());
	glUniform4fv(index, 1, ValuePointer(value));
}

void GMGLShaderProgram::setVec3(GMint32 index, const GMfloat value[3])
{
	GM_ASSERT(verify());
	glUniform3fv(index, 1, value);
}

void GMGLShaderProgram::setInt(GMint32 index, GMint32 value)
{
	GM_ASSERT(verify());
	glUniform1i(index, value);
}

void GMGLShaderProgram::setFloat(GMint32 index, GMfloat value)
{
	GM_ASSERT(verify());
	glUniform1f(index, value);
}

void GMGLShaderProgram::setBool(GMint32 index, bool value)
{
	setInt(index, (GMint32)value);
}

bool GMGLShaderProgram::setInterfaceInstance(const GMString& interfaceName, const GMString& instanceName, GMShaderType type)
{
	GLenum shaderType = GMGLShaderInfo::toGLShaderType(type);
	return setSubrotinue(interfaceName, instanceName, shaderType);
}

bool GMGLShaderProgram::setSubrotinue(const GMString& interfaceName, const GMString& implement, GMuint32 shaderType)
{
	GM_ASSERT(verify());
	D(d);
	if (!d->techniqueIndex)
		d->techniqueIndex = getIndex(interfaceName);
	setInt(d->techniqueIndex, toTechniqueEntranceId(implement));
	return true;
}

bool GMGLShaderProgram::verify()
{
	D(d);
	return d->lastUsedProgram == d->shaderProgram;
}

bool GMGLShaderProgram::load()
{
	D(d);
	if (d->shaderInfos.size() == 0)
		return false;

	GLuint program = glCreateProgram();
	setProgram(program);

	for (auto& entry : d->shaderInfos)
	{
		GLuint shader = glCreateShader(entry.type);
		d->shaders.push_back(shader);

		expandSource(entry); // 展开glsl
		const std::string& src = entry.source.toStdString();
		const GLchar* source = src.c_str();
		if (!source)
		{
			removeShaders();
			return false;
		}

		glShaderSource(shader, 1, &source, NULL);
		glCompileShader(shader);

		GLint compiled;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (!compiled)
		{
			GLsizei len;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);

			GLchar* log = new GLchar[len + 1];
			glGetShaderInfoLog(shader, len, &len, log);

			GMStringReader reader(entry.source);
			std::string report;
			GMint32 ln = 0;
			auto iter = reader.lineBegin();
			do
			{
				report += std::to_string(++ln) + ":\t" + (*iter).toStdString();
				iter++;
			} while (iter.hasNextLine());

			gm_error(gm_dbg_wrap("Shader source: \n{0}"), report);
			gm_error(gm_dbg_wrap("Shader compilation failed: {0}"), log);
			gm_error(gm_dbg_wrap("filename: {0} {1}"), GM_CRLF, GMString(entry.filename));
			GM_ASSERT(false);
			GMMessage crashMsg(GameMachineMessageType::CrashDown);
			GM.postMessage(crashMsg);
			GM_delete_array(log);
			return false;
		}

		glAttachShader(program, shader);
	}

	glLinkProgram(program);

	GLint linked = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		GLsizei len;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);

		GLchar* log = new GLchar[len + 1];
		glGetProgramInfoLog(program, len, &len, log);
		gm_error(log);
		GM_ASSERT(false);
		GMMessage crashMsg(GameMachineMessageType::CrashDown);
		GM.postMessage(crashMsg);
		GM_delete_array(log);

		removeShaders();
		return false;
	}

	return true;
}

void GMGLShaderProgram::removeShaders()
{
	D(d);
	GMGLShaderIDList& shadersInfo = d->shaders;
	for (auto& shader : shadersInfo)
	{
		glDeleteShader(shader);
	}
}

void GMGLShaderProgram::expandSource(REF GMGLShaderInfo& shaderInfo)
 {
	D(d);
	// 解析源码，展开gm特有的宏
	shaderInfo.source = expandSource(shaderInfo.filename, shaderInfo.source);
}

GMString GMGLShaderProgram::expandSource(const GMString& filename, const GMString& source)
{
	GMStringReader reader(source);
	GMString n, line;
	GMString expanded;
	auto iter = reader.lineBegin();
	while (true)
	{
		line = *iter;
		if (matchMacro(line, "include", n))
			expandInclude(filename, n, line);
		else if (matchMacro(line, "alias", n))
			expandAlias(n, line);
		expanded += replaceLine(line);
		if (!iter.hasNextLine())
			break;
		++iter;
	}
	return expanded;
}

bool GMGLShaderProgram::matchMacro(const GMString& source, const GMString& macro, REF GMString& result)
{
	std::string s = source.toStdString();
	std::string expr = "#(\\s*)" + macro.toStdString() + "(\\s+)(.*)";
	std::smatch match;
	if (std::regex_search(s, match, std::regex(expr.c_str())))
	{
		GM_ASSERT(match.size() >= 4);
		result = match[3].str();
		return true;
	}
	return false;
}

void GMGLShaderProgram::expandInclude(const GMString& workingDir, const GMString& fn, IN OUT GMString& source)
{
	std::string f;
	std::string s = source.toStdString();
	static std::string expr = "(\\s*)\"(.+)\"";
	std::smatch match;
	if (std::regex_search(s, match, std::regex(expr.c_str())))
	{
		if (match.size() != 3)
			return;
		f = match[2].str();
	}

	GMString dir = GMPath::directoryName(workingDir);
	GMString include = dir + f;
	GMBuffer buf;
	if (GM.getGamePackageManager()->readFileFromPath(include, &buf))
	{
		buf.convertToStringBuffer();
		source = expandSource(include, GMString((char*)buf.buffer)) + GM_CRLF;
	}
	else
	{
		gm_warning(gm_dbg_wrap("GMGLShaderProgram::expandInclude: GL shader '{0}' not found, use empty file instead!"), include);
	}
}

void GMGLShaderProgram::expandAlias(const GMString& alias, IN OUT GMString& source)
{
	D(d);
	std::string s = alias.toStdString();
	static std::string expr = "(\\s*)(\\S+)(\\s+)(.+)";
	std::smatch match;
	if (std::regex_search(s, match, std::regex(expr.c_str())))
	{
		GM_ASSERT(match.size() == 5);
		std::string replacement = match[2].str();
		std::string a = match[4].str();
		d->aliasMap.insert({ "${" + replacement + "}", a });
	}
	source = L"";
}

GMString& GMGLShaderProgram::replaceLine(IN OUT GMString& line)
{
	D(d);
	for (auto& alias : d->aliasMap)
	{
		line = line.replace(alias.first, alias.second);
	}
	return line;
}

bool GMGLShaderProgram::setInterface(GameMachineInterfaceID id, void* in)
{
	return false;
}

bool GMGLShaderProgram::getInterface(GameMachineInterfaceID id, void** out)
{
	return false;
}