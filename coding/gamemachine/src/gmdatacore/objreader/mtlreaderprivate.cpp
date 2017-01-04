﻿#include "stdafx.h"
#include "mtlreaderprivate.h"
#include "objreaderprivate.h"
#include "utilities/scanner.h"
#include "gmdatacore/imagereader/imagereader.h"

#define KW_REMARK "#"
#define KW_NEWMATERIAL "newmtl"
#define KW_NS "Ns"
#define KW_D "d"
#define KW_TR "Tr"
#define KW_TF "Tf"
#define KW_ILLUM "illum"
#define KW_KA "Ka"
#define KW_KD "Kd"
#define KW_KS "Ks"
#define KW_MAP_KD "map_Kd"

static bool strEqual(const char* str1, const char* str2)
{
	return !strcmp(str1, str2);
}

Materials& MtlReaderPrivate::getMaterials()
{
	return m_materials;
}

void MtlReaderPrivate::parseLine(const char* line)
{
	Scanner scanner(line);
	char command[LINE_MAX];
	scanner.next(command);

	if (strEqual(command, KW_REMARK))
		return;

	if (strEqual(command, KW_NEWMATERIAL))
	{
		char name[LINE_MAX];
		scanner.next(name);
		m_pCurrentMaterial = &(m_materials[name]);
	}
	else if (strEqual(command, KW_NS))
	{
		GMfloat value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->Ns = value;
	}
	else if (strEqual(command, KW_D))
	{
		GMfloat value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->d = value;
	}
	else if (strEqual(command, KW_TR))
	{
		GMfloat value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->Tr = value;
	}
	else if (strEqual(command, KW_TF))
	{
		GMfloat value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->Tf_r = value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->Tf_g = value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->Tf_b = value;
	}
	else if (strEqual(command, KW_ILLUM))
	{
		GMint value;
		scanner.nextInt(&value);
		m_pCurrentMaterial->illum = value;
	}
	else if (strEqual(command, KW_KA))
	{
		GMfloat value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->Ka_r = value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->Ka_g = value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->Ka_b = value;
		m_pCurrentMaterial->Ka_switch = true;
	}
	else if (strEqual(command, KW_KD))
	{
		GMfloat value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->Kd_r = value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->Kd_g = value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->Kd_b = value;
		m_pCurrentMaterial->Kd_switch = true;
	}
	else if (strEqual(command, KW_KS))
	{
		GMfloat value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->Ks_r = value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->Ks_g = value;
		scanner.nextFloat(&value);
		m_pCurrentMaterial->Ks_b = value;
		m_pCurrentMaterial->Ks_switch = true;
	}
	else if (strEqual(command, KW_MAP_KD))
	{
		ASSERT(false);
		// 暂时不支持obj导入纹理，如果要导入，要考虑内存释放
	}
}