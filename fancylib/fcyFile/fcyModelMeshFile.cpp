#include "fcyModelMeshFile.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

fcyRefPointer<fcyModelLabel> fcyModelMeshFile::FindLabel(fuLong LabelName)
{
	for(fuInt i = 0; i<m_Labels.size(); ++i)
	{
		if(m_Labels[i]->GetLabelName() == LabelName)
			return m_Labels[i];
	}

	return NULL;
}

void fcyModelMeshFile::Load(fcyStream* pStream, fBool IgnoreUnsupportLabel)
{
	fcyBinaryReader tReader(pStream);
	
	// 清空数据
	m_Author.clear();
	m_Description.clear();
	m_Labels.clear();

	// 读取文件头并验证
	if(FMM_MAKE_LABELNAME4("FMM\0") != tReader.ReadUInt32())
		throw fcyException("fcyModelMeshFile::Load", "File format error.");
	if(Version != tReader.ReadUInt32())
		throw fcyException("fcyModelMeshFile::Load", "Version not support.");
	fuInt tBlockCount = tReader.ReadUInt32();
	m_Author = fcyModelLabel::ReadString(pStream);
	m_Description = fcyModelLabel::ReadString(pStream);

	// 读取块
	for(fuInt i = 0; i<tBlockCount; ++i)
	{
		fuLong tLable = tReader.ReadUInt64(); // 块标签头
		fuInt tLen = tReader.ReadUInt32();    // 块大小

		// 记录当前的读取位置
		fLen tPos = pStream->GetPosition();

		// 读取标签
		fcyModelLabelFactory::CreatorFunction* tFunc = fcyModelLabel::GetFactory().GetCreator(tLable);
		if(!tFunc)
		{
			if(IgnoreUnsupportLabel)
			{
				// 跳过不识别的块
				pStream->SetPosition(FCYSEEKORIGIN_CUR, tLen);
			}
			else
				throw fcyException("fcyModelMeshFile::Load", "Unsupport label found.");
		}
		else
		{
			// 读取块
			fcyRefPointer<fcyModelLabel> pLabel = tFunc->CreateInstance();
			pLabel->ReadData(pStream);

			m_Labels.push_back(pLabel);

			// 获得当前位置
			fuInt tBlockLen = (fuInt)(pStream->GetPosition() - tPos);
			if(tBlockLen != tLen)
				throw fcyException("fcyModelMeshFile::Load", "Error detected when read block.");
		}
	}
}


void fcyModelMeshFile::Save(fcyStream* pStream)
{
	fcyBinaryWriter tWritter(pStream);

	// 写入文件头
	tWritter.Write((fuInt)FMM_MAKE_LABELNAME4("FMM\0"));
	tWritter.Write((fuInt)Version);
	tWritter.Write((fuInt)m_Labels.size());
	fcyModelLabel::WriteString(pStream, m_Author);
	fcyModelLabel::WriteString(pStream, m_Description);

	// 写入块
	for(fuInt i = 0; i<m_Labels.size(); ++i)
	{
		// 写入标签头
		tWritter.Write((fuLong)m_Labels[i]->GetLabelName());
		
		// 块大小
		tWritter.Write((fuInt)0);

		// 获得当前位置
		fLen tStartOfBlockPos = pStream->GetPosition();

		// 写入数据
		m_Labels[i]->WriteData(pStream);

		// 读取当前位置
		fLen tEndOfBlockPos = pStream->GetPosition();
		fLen tLen = tEndOfBlockPos - tStartOfBlockPos;

		// 跳转并写入数据
		pStream->SetPosition(FCYSEEKORIGIN_BEG, tStartOfBlockPos - 4);
		tWritter.Write((fuInt)tLen);

		// 返回到下一个标签处
		pStream->SetPosition(FCYSEEKORIGIN_BEG, tEndOfBlockPos);
	}
}
