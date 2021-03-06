﻿#include "stdafx.h"
#include <GL/glew.h>
#include <stdio.h>
#include "gmglshaderprogram.h"
#include "gmglgraphic_engine.h"
#include <regex>
#include "foundation/gamemachine.h"
#include <linearmath.h>
#include "gmglhelper.h"

#pragma warning (disable: 4302)
#pragma warning (disable: 4311)
#pragma warning (disable: 4312)

BEGIN_NS

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
			BlendFilter = 6,
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

GM_PRIVATE_OBJECT_UNALIGNED(GMGLShaderProgram)
{
	GM_DECLARE_PUBLIC(GMGLShaderProgram)
	const IRenderContext* context = nullptr;
	GMGLShaderInfos shaderInfos;
	GMGLShaderIDList shaders;
	GMuint32 shaderProgram = 0;
	GMuint32 techniqueIndex = 0;
	HashMap<GMString, GMString, GMStringHashFunctor> aliasMap;
	Set<GMString> unusedVariableNames;
	Array<HashMap<GMString, GMString, GMStringHashFunctor>, (GMsize_t)GMShaderType::Geometry> definesMap;

	void setProgram(GMuint32 program);
	void removeShaders();
	bool setSubrotinue(const GMString& interfaceName, const GMString& implement, GMuint32 shaderType);
	bool verify();
};

bool GMGLShaderProgramPrivate::setSubrotinue(const GMString& interfaceName, const GMString& implement, GMuint32 shaderType)
{
	GM_ASSERT(verify());
	P_D(pd);
	if (!techniqueIndex)
		techniqueIndex = pd->getIndex(interfaceName);
	pd->setInt(techniqueIndex, toTechniqueEntranceId(implement));
	return true;
}

bool GMGLShaderProgramPrivate::verify()
{
	// Always valid
	return true;
}

void GMGLShaderProgramPrivate::setProgram(GMuint32 program)
{
	GM_ASSERT(program);
	shaderProgram = program;
}

void GMGLShaderProgramPrivate::removeShaders()
{
	GMGLShaderIDList& shadersInfo = shaders;
	for (auto& shader : shadersInfo)
	{
		glDeleteShader(shader);
	}
}

GMGLShaderProgram::GMGLShaderProgram(const IRenderContext* context)
{
	GM_CREATE_DATA();
	GM_SET_PD();

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
	glUseProgram(d->shaderProgram);

	GMGLGraphicEngine* engine = gm_cast<GMGLGraphicEngine*>(d->context->getEngine());
	engine->shaderProgramChanged(this);
}

void GMGLShaderProgram::attachShader(const GMGLShaderInfo& shaderCfgs)
{
	D(d);
	d->shaderInfos.push_back(shaderCfgs);
}


void GMGLShaderProgram::setDefinesMap(GMShaderType t, const HashMap<GMString, GMString, GMStringHashFunctor>& definesMap)
{
	D(d);
	GMsize_t index = static_cast<GMsize_t>(t) - 1;
	d->definesMap[index] = definesMap;
}

GMint32 GMGLShaderProgram::getIndex(const GMString& name)
{
	GMint32 id = glGetUniformLocation(getProgram(), name.toStdString().c_str());
	if (id == -1)
	{
		D(d);
		if (d->unusedVariableNames.find(name) == d->unusedVariableNames.end())
		{
			gm_warning(gm_dbg_wrap("Cannot get location of string '{0}' in glsl. "
			"If you don't need this variable in your shader, just use '#unused YOUR_VARIABLE_NAME' to suppress this warning."), name);
		}
	}
	return id;
}

void GMGLShaderProgram::setMatrix4(GMint32 index, const GMMat4& value)
{
	D(d);
	GM_ASSERT(d->verify());
	glUniformMatrix4fv(index, 1, GL_FALSE, ValuePointer(value));
}

void GMGLShaderProgram::setVec4(GMint32 index, const GMFloat4& value)
{
	D(d);
	GM_ASSERT(d->verify());
	glUniform4fv(index, 1, ValuePointer(value));
}

void GMGLShaderProgram::setVec3(GMint32 index, const GMfloat value[3])
{
	D(d);
	GM_ASSERT(d->verify());
	glUniform3fv(index, 1, value);
}

void GMGLShaderProgram::setInt(GMint32 index, GMint32 value)
{
	D(d);
	GM_ASSERT(d->verify());
	glUniform1i(index, value);
}

void GMGLShaderProgram::setFloat(GMint32 index, GMfloat value)
{
	D(d);
	GM_ASSERT(d->verify());
	glUniform1f(index, value);
}

void GMGLShaderProgram::setBool(GMint32 index, bool value)
{
	setInt(index, (GMint32)value);
}

bool GMGLShaderProgram::setInterfaceInstance(const GMString& interfaceName, const GMString& instanceName, GMShaderType type)
{
	D(d);
	GLenum shaderType = GMGLShaderInfo::toGLShaderType(type);
	return d->setSubrotinue(interfaceName, instanceName, shaderType);
}

bool GMGLShaderProgram::load()
{
	D(d);
	if (d->shaderInfos.size() == 0)
		return false;

	GLuint program = glCreateProgram();
	d->setProgram(program);

	constexpr GMsize_t shaderTypes = static_cast<GMsize_t>(GMShaderType::Geometry);
	Array<Vector<GMString>, shaderTypes> contents;
	for (auto& entry : d->shaderInfos)
	{
		GMsize_t index = static_cast<GMsize_t>(entry.fromGLShaderType(entry.type)) - 1;
		contents[index].push_back(entry.source + GM_NEXTLINE);
	}

	for (GMsize_t index = 0; index < shaderTypes; ++index)
	{
		if (contents[index].empty())
			continue;

		GLuint type = GMGLShaderInfo::toGLShaderType(static_cast<GMShaderType>(index + 1));
		GLuint shader = glCreateShader(type);
		d->shaders.push_back(shader);

		const GLchar* version = !GMGLHelper::isOpenGLShaderLanguageES() ? "#version 330\n" : "#version 300 es\n";
		Vector<const GLchar*> sourcesVector = { version };
		GMint32 fileOffset = 0;

#if GM_RASPBERRYPI
		sourcesVector.push_back("#define GM_RASPBERRYPI 1\n");
		fileOffset = 1;
#endif
		GMString definesString;
		for (auto& definesMap : d->definesMap[index])
		{
			definesString += "#define " + definesMap.first + " " + definesMap.second + GM_NEXTLINE;
		}

		Vector<std::string> codeCache;
		if (!definesString.isEmpty())
			codeCache.push_back(definesString.toStdString());

		for (const auto& src : contents[index])
		{
			codeCache.push_back(src.toStdString());
		}

		for (const auto& src : codeCache)
		{
			sourcesVector.push_back(src.c_str());
		}

		glShaderSource(shader, gm_sizet_to_int(sourcesVector.size()), sourcesVector.data(), NULL);
		glCompileShader(shader);

		GLint compiled;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

		if (!compiled)
		{
			GLsizei len;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);

			GLchar* log = new GLchar[len + 1];
			glGetShaderInfoLog(shader, len, &len, log);

			GMint32 ln = 0, fn = fileOffset;
			for (const GMString& src : contents[index])
			{
				GMStringReader reader(src);
				std::string line;
				auto iter = reader.lineBegin();
				do
				{
					line += std::to_string(++ln) + ":\t" + (*iter).toStdString();
					iter++;
				} while (iter.hasNextLine());
				line += std::to_string(++ln) + ":\t" + (*iter).toStdString();
				gm_error(gm_dbg_wrap("Shader source of file {0}: {1}{2}"), GMString(++fn), GMString(GM_NEXTLINE), line);
				ln = 0;
			}
			gm_error(gm_dbg_wrap("Shader compilation failed: {0}"), log);
			GM_ASSERT(false);
			GMMessage crashMsg(GameMachineMessageType::CrashDown);
			GM.postMessage(crashMsg);
			GM_delete_array(log);
			d->removeShaders();
			return false;
		}
		else
		{
#if GM_DEBUG
			GLint ss;
			glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &ss);
			GLchar* _src = new GLchar[ss]; // 1MB source
			GLsizei len;
			glGetShaderSource(shader, ss, &len, _src);
			gm_debug(gm_dbg_wrap("Source: {0}"), GMString(_src));
			GM_delete_array(_src);
#endif
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
		gm_error(gm_dbg_wrap("{0}"), GMString(log));
		GM_ASSERT(false);
		GMMessage crashMsg(GameMachineMessageType::CrashDown);
		GM.postMessage(crashMsg);
		GM_delete_array(log);

		d->removeShaders();
		return false;
	}

	return true;
}

GMuint32 GMGLShaderProgram::getProgram()
{
	D(d);
	GM_ASSERT(d->shaderProgram);
	return d->shaderProgram;
}

bool GMGLShaderProgram::setInterface(GameMachineInterfaceID id, void* in)
{
	return false;
}

bool GMGLShaderProgram::getInterface(GameMachineInterfaceID id, void** out)
{
	return false;
}

GM_PRIVATE_OBJECT_UNALIGNED(GMGLComputeShaderProgram)
{
	const IRenderContext* context = nullptr;
	GMuint32 shaderProgram = 0;
	GMuint32 shaderId = 0;
	GMuint32 boBase = 0;
};

GMGLComputeShaderProgram::GMGLComputeShaderProgram(const IRenderContext* context)
{
	GM_CREATE_DATA();
	D(d);
	d->context = context;
}

GMGLComputeShaderProgram::~GMGLComputeShaderProgram()
{

}

void GMGLComputeShaderProgram::dispatch(GMint32 threadGroupCountX, GMint32 threadGroupCountY, GMint32 threadGroupCountZ)
{
	D(d);
	glUseProgram(d->shaderProgram);
	glDispatchCompute(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	cleanUp();
}

void GMGLComputeShaderProgram::load(const GMString& path, const GMString& source, const GMString& entryPoint)
{
	D(d);
	d->shaderProgram = glCreateProgram();
	if (d->shaderProgram)
	{
		GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
		d->shaderId = shader;

		const std::string& src = source.toStdString();
		const GLchar* source = src.c_str();
		if (!source)
		{
			return dispose();
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

			GMStringReader reader(source);
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
			gm_error(gm_dbg_wrap("filename: {0} {1}"), GM_NEXTLINE, path);
			GM_ASSERT(false);
			GMMessage crashMsg(GameMachineMessageType::CrashDown);
			GM.postMessage(crashMsg);
			GM_delete_array(log);
			dispose();
		}

		glAttachShader(d->shaderProgram, shader);
		glLinkProgram(d->shaderProgram);

		GLint linked = GL_FALSE;
		glGetProgramiv(d->shaderProgram, GL_LINK_STATUS, &linked);
		if (!linked)
		{
			GLsizei len;
			glGetProgramiv(d->shaderProgram, GL_INFO_LOG_LENGTH, &len);

			GLchar* log = new GLchar[len + 1];
			glGetProgramInfoLog(d->shaderProgram, len, &len, log);
			gm_error(log);
			GM_ASSERT(false);
			GMMessage crashMsg(GameMachineMessageType::CrashDown);
			GM.postMessage(crashMsg);
			GM_delete_array(log);
			dispose();
		}
	}
}

bool GMGLComputeShaderProgram::createReadOnlyBufferFrom(GMComputeBufferHandle bufferSrc, OUT GMComputeBufferHandle* bufferOut)
{
	GMuint32 src = (GMuint32)bufferSrc;
	GMGLBeginGetErrorsAndCheck();
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, src);
	GMint32 size = 0;
	glGetBufferParameteriv(GL_SHADER_STORAGE_BUFFER, GL_BUFFER_SIZE, &size);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// 创建新buffer
	createBuffer(size, 1, NULL, GMComputeBufferType::Constant, bufferOut);
	GMGLEndGetErrorsAndCheck();
	return (*bufferOut) != 0;
}

bool GMGLComputeShaderProgram::createBuffer(GMuint32 elementSize, GMuint32 count, void* pInitData, GMComputeBufferType type, OUT GMComputeBufferHandle* ppBufOut)
{
	GMuint32 buf = 0;
	GMGLBeginGetErrorsAndCheck();
	glGenBuffers(1, &buf);

	if (type == GMComputeBufferType::Structured || type == GMComputeBufferType::UnorderedStructured)
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, buf);
		glBufferData(GL_SHADER_STORAGE_BUFFER, elementSize * count, pInitData, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
	else
	{
		GM_ASSERT(type == GMComputeBufferType::Constant);
		glBindBuffer(GL_UNIFORM_BUFFER, buf);
		glBufferData(GL_UNIFORM_BUFFER, elementSize * count, pInitData, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	GMGLEndGetErrorsAndCheck();
	*ppBufOut = (GMComputeBufferHandle) buf;
	return buf != 0;
}

void GMGLComputeShaderProgram::release(GMComputeBufferHandle handle)
{
	GMuint32 bufferId = (GMuint32)handle;
	if (glIsBuffer(bufferId))
		glDeleteBuffers(1, &bufferId);
}

bool GMGLComputeShaderProgram::createBufferShaderResourceView(GMComputeBufferHandle handle, OUT GMComputeSRVHandle* out)
{
	if (out)
		*out = handle;
	return true;
}

bool GMGLComputeShaderProgram::createBufferUnorderedAccessView(GMComputeBufferHandle handle, OUT GMComputeUAVHandle* out)
{
	if (out)
		*out = handle;
	return true;
}

void GMGLComputeShaderProgram::bindShaderResourceView(GMuint32 cnt, GMComputeSRVHandle* srvs)
{
	D(d);
	for (GMuint32 i = 0; i < cnt; ++i)
	{
		GMuint32 buf = (GMuint32)srvs[i];
		GMGLBeginGetErrorsAndCheck();
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, buf);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, d->boBase++, buf);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		GMGLEndGetErrorsAndCheck();
	}
}

void GMGLComputeShaderProgram::bindUnorderedAccessView(GMuint32 cnt, GMComputeUAVHandle* uavs)
{
	bindShaderResourceView(cnt, uavs);
}

void GMGLComputeShaderProgram::bindConstantBuffer(GMComputeBufferHandle handle)
{
	D(d);
	GMuint32 buf = (GMuint32)handle;
	GMGLBeginGetErrorsAndCheck();
	glBindBuffer(GL_UNIFORM_BUFFER, buf);
	glBindBufferBase(GL_UNIFORM_BUFFER, d->boBase++, buf);
	GMGLEndGetErrorsAndCheck();
}

void GMGLComputeShaderProgram::setBuffer(GMComputeBufferHandle handle, GMComputeBufferType type, void* dataPtr, GMuint32 sz)
{
	D(d);
	GMuint32 buf = (GMuint32) handle;
	GMGLBeginGetErrorsAndCheck();

	GLenum target = type == GMComputeBufferType::Constant ? GL_UNIFORM_BUFFER : GL_SHADER_STORAGE_BUFFER;
	glBindBuffer(target, buf);
	glBufferSubData(target, 0, sz, dataPtr);
	glBindBuffer(target, 0);
	GMGLEndGetErrorsAndCheck();
}

void GMGLComputeShaderProgram::copyBuffer(GMComputeBufferHandle destHandle, GMComputeBufferHandle srcHandle)
{
	GMuint32 dest = (GMuint32)destHandle;
	GMuint32 src = (GMuint32)srcHandle;
	GMint32 size = 0;
	GMGLBeginGetErrorsAndCheck();
	glBindBuffer(GL_COPY_READ_BUFFER, src);
	glGetBufferParameteriv(GL_COPY_READ_BUFFER, GL_BUFFER_SIZE, &size);

	glBindBuffer(GL_COPY_WRITE_BUFFER, dest);

	GMint32 usage = GL_DYNAMIC_DRAW;
	glGetBufferParameteriv(GL_COPY_WRITE_BUFFER, GL_BUFFER_USAGE, &usage);
	glBufferData(GL_COPY_WRITE_BUFFER, size, nullptr, usage);
	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, size);

	GMGLEndGetErrorsAndCheck();
}

void* GMGLComputeShaderProgram::mapBuffer(GMComputeBufferHandle handle)
{
	GMuint32 buf = (GMuint32)handle;
	GMGLBeginGetErrorsAndCheck();
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buf);
	void* data = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	GMGLEndGetErrorsAndCheck();
	return data;
}

void GMGLComputeShaderProgram::unmapBuffer(GMComputeBufferHandle handle)
{
	GMGLBeginGetErrorsAndCheck();
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	GMGLEndGetErrorsAndCheck();
}


bool GMGLComputeShaderProgram::canRead(GMComputeBufferHandle handle)
{
	return true;
}


GMsize_t GMGLComputeShaderProgram::getBufferSize(GMComputeBufferType type, GMComputeBufferHandle handle)
{
	GMint32 sz = 0;
	GMuint32 buf = (GMuint32)handle;
	GLenum target = type == GMComputeBufferType::Constant ? GL_UNIFORM_BUFFER : GL_SHADER_STORAGE_BUFFER;
	glBindBuffer(target, buf);
	glGetBufferParameteriv(target, GL_BUFFER_SIZE, &sz);
	glBindBuffer(target, 0);
	return static_cast<GMsize_t>(sz);
}

bool GMGLComputeShaderProgram::setInterface(GameMachineInterfaceID id, void* in)
{
	return false;
}

bool GMGLComputeShaderProgram::getInterface(GameMachineInterfaceID id, void** out)
{
	return false;
}

void GMGLComputeShaderProgram::dispose()
{
	D(d);
	glDeleteShader(d->shaderId);
	glDeleteProgram(d->shaderProgram);
}

void GMGLComputeShaderProgram::cleanUp()
{
	D(d);
	d->boBase = 0;
}

END_NS
