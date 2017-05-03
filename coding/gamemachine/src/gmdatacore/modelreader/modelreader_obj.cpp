﻿#include "stdafx.h"
#include "modelreader_obj.h"
#include "gmdatacore/object.h"
#include "gmdatacore/gamepackage.h"
#include "utilities/scanner.h"

#define RESERVED 4096
#define INVALID -1

static bool slashPredicate(char c)
{
	return c == '/';
}

class StringReader
{
public:
	StringReader(const char* string)
		: m_string(string)
		, m_p(string)
	{
	}

	bool readLine(char* line)
	{
		GMint offset = 0;
		while (*m_p != '\r' && *m_p != '\n')
		{
			if (!*m_p)
			{
				line[offset] = 0;
				return false;
			}
			line[offset++] = *(m_p++);
		}

		m_p++;
		line[offset] = 0;
		return true;
	}

private:
	const char* m_string;
	const char* m_p;
};

template <GMint D, typename T>
static void pushBackData(Scanner& scanner, std::vector<T>& container)
{
	T data;
	for (GMint i = 0; i < D; i++)
	{
		scanner.nextFloat(&(data[i]));
	}
	container.push_back(data);
}

static void pushBackData(const vmath::vec3 extents, const vmath::vec3& position, Scanner& scanner, std::vector<vmath::vec3>& container)
{
	vmath::vec3 data;
	for (GMint i = 0; i < 3; i++)
	{
		GMfloat d;
		scanner.nextFloat(&d);
		d = d * extents[i] + position[i];
		data[i] = d;
	}
	container.push_back(data);
}

ModelReader_Obj::ModelReader_Obj()
{
	init();
}

void ModelReader_Obj::init()
{
	D(d);
	d = ModelReader_ObjPrivate();
	d.object = nullptr;
	d.currentComponent = nullptr;
}

bool ModelReader_Obj::load(const ModelLoadSettings& settings, GamePackageBuffer& buffer, OUT Object** object)
{
	D(d);
	init();

	buffer.convertToStringBuffer();
	char line[LINE_MAX];
	StringReader sr((char*)buffer.buffer);

	d.object = new Object;
	ChildObject* child = new ChildObject();
	d.object->append(child);

	// 事先分配一些内存，提高效率
	d.vertices.reserve(RESERVED);
	d.textures.reserve(RESERVED);
	d.normals.reserve(RESERVED);

	while (sr.readLine(line))
	{
		Scanner s(line);
		char token[LINE_MAX];
		s.next(token);

		if (strEqual(token, "#"))
		{
			// comment
		}
		else if (strEqual(token, "v"))
		{
			// vertex
			pushBackData(settings.extents, settings.position, s, d.vertices);
		}
		else if (strEqual(token, "vn"))
		{
			// normal
			pushBackData<3>(s, d.normals);
		}
		else if (strEqual(token, "vt"))
		{
			// texture
			pushBackData<2>(s, d.textures);
		}
		else if (strEqual(token, "mtllib"))
		{
			// material
			char name[LINE_MAX];
			s.next(name);
			loadMaterial(settings, name);
		}
		else if (strEqual(token, "usemtl"))
		{
			// use material
			char name[LINE_MAX];
			s.next(name);
			d.currentMaterialName = name;
			if (d.currentComponent)
				child->appendComponent(d.currentComponent);
			d.currentComponent = nullptr;
		}
		else if (strEqual(token, "f"))
		{
			// face
			appendFace(s);
		}
	}

	if (d.currentComponent)
		child->appendComponent(d.currentComponent);
	*object = d.object;

	return true;
}

bool ModelReader_Obj::test(const GamePackageBuffer& buffer)
{
	return buffer.buffer[0] == '#';
}

void ModelReader_Obj::appendFace(Scanner& scanner)
{
	D(d);
	const ModelReader_Obj_Material& material = d.materials[d.currentMaterialName];

	ChildObject* child = d.object->getChildObjects()[0];
	if (!d.currentComponent)
		d.currentComponent = new Component(child);
	// setup component by material

	char face[LINE_MAX];
	d.currentComponent->beginFace();
	while (true)
	{
		scanner.next(face);
		if (!strlen(face))
			break;

		Scanner faceScanner(face, false, slashPredicate);

		GMint v = INVALID, t = INVALID, n = INVALID;
		faceScanner.nextInt(&v);
		faceScanner.nextInt(&t);
		faceScanner.nextInt(&n);

		ASSERT(v != INVALID);
		{
			auto& vec = d.vertices[v - 1];
			d.currentComponent->vertex(vec[0], vec[1], vec[2]);
		}

		{
			auto& vec = t != INVALID ? d.textures[t - 1] : vmath::vec2(0, 0);
			d.currentComponent->uv(vec[0], vec[1]);
		}

		ASSERT(n != INVALID);
		{
			auto& vec = d.normals[n - 1];
			d.currentComponent->normal(vec[0], vec[1], vec[2]);
		}
	};
	d.currentComponent->endFace();
}

void ModelReader_Obj::loadMaterial(const ModelLoadSettings& settings, const char* mtlFilename)
{
	D(d);
	std::string mtlPath = settings.modelName;
	mtlPath.append("/");
	mtlPath.append(mtlFilename);
	std::string mtlFullPath = settings.gamePackage.path(PI_MODELS, mtlPath.c_str());
	GamePackageBuffer buffer;
	settings.gamePackage.readFileFromPath(mtlFullPath.c_str(), &buffer);
	buffer.convertToStringBuffer();
	char line[LINE_MAX];
	StringReader sr((char*)buffer.buffer);

	ModelReader_Obj_Material* material = nullptr;
	while (sr.readLine(line))
	{
		Scanner s(line);
		char token[LINE_MAX];
		s.next(token);
		if (strEqual(token, "newmtl"))
		{
			char name[LINE_MAX];
			s.next(name);
			material = &(d.materials[name]);
			memset(material, 0, sizeof(*material));
		}
		else if (strEqual(token, "Ns"))
		{
			ASSERT(material);
			s.nextFloat(&material->ns);
		}
		else if (strEqual(token, "Kd"))
		{
			ASSERT(material);
			s.nextFloat(&material->kd[0]);
			s.nextFloat(&material->kd[1]);
			s.nextFloat(&material->kd[2]);
		}
		else if (strEqual(token, "Ka"))
		{
			ASSERT(material);
			s.nextFloat(&material->ka[0]);
			s.nextFloat(&material->ka[1]);
			s.nextFloat(&material->ka[2]);
		}
		else if (strEqual(token, "Ks"))
		{
			ASSERT(material);
			s.nextFloat(&material->ks[0]);
			s.nextFloat(&material->ks[1]);
			s.nextFloat(&material->ks[2]);
		}
	}
}