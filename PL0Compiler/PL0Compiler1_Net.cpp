// PL0Compiler1_Net.cpp : �������̨Ӧ�ó������ڵ�.
//

#include "stdafx.h"
#include "stdio.h"
#include "string.h"

// 当启用 Web GUI 版本时，不编译命令行入口，避免重复的 main
#ifdef PL0_ENABLE_WEB_GUI
#define PL0_NO_MAIN
#endif

//enum	WORD_TYPE_ENUM;

void InitializeSingleCharacterTable();
void	InitializeReservedWordTable();
void PrintSourceFile();
int	 Initialization();
void LexicalAnalysis();
int ParsingAnalysis();
int GenerateAnalysis();
void Interpreter();

//#define		SOURCE_FILE_NAME				"Test0.pl"
#define SOURCE_FILE_NAME "/Users/ruiningyang/CodeStudy/PL0Compiler/PL0Compiler/PL0Compiler/Test1.pl"
//#define		SOURCE_FILE_NAME				"C:\\Users\\RuiningYang\\Desktop\\code\\PL0_NET_sourcecode\\PL0_NET\\Test0.pl"
//#define		SOURCE_FILE_NAME				"E:\\�����������\\garnet0o0\\garnet0o0\\PL0_NET_sourcecode\\PL0_NET\\Test0.pl"
//#define		SOURCE_FILE_NAME				"SourceFile1.pl0"

FILE*														fpSourceFile;
char                                                        g_szSourceFileName[260]=SOURCE_FILE_NAME;

// 允许外部设置源文件路径（用于 HTTP/GUI 入口）
void SetSourceFileName(const char* szPath)
{
	if(szPath && szPath[0]!='\0')
	{
		strncpy(g_szSourceFileName, szPath, sizeof(g_szSourceFileName)-1);
		g_szSourceFileName[sizeof(g_szSourceFileName)-1] = '\0';
	}
}

#ifndef PL0_NO_MAIN
int main(int argc, char* argv[])
{
	if(Initialization()!=OK)
	{
		printf("Initialization failed\n");
		return 1;
	}
	PrintSourceFile();
	getchar();//����ͣ�����ȴ�����һ���ַ�
	
	LexicalAnalysis();
	getchar();//����ͣ�����ȴ�����һ���ַ�

	if(ParsingAnalysis()!=OK)
	{
		printf("Parsing failed\n");
		return 1;
	}
	getchar();//����ͣ�����ȴ�����һ���ַ�

	if(GenerateAnalysis()!=OK)
	{
		printf("Generate failed\n");
		return 1;
	}
	getchar();//����ͣ�����ȴ�����һ���ַ�

	Interpreter();
	getchar();//����ͣ�����ȴ�����һ���ַ�

	return 0;
}
#endif

int Initialization()
{
	// 采用足够长的缓冲区以避免长路径被截断（如 Xcode DerivedData 下的可执行目录）
	char	szFileName[260];

	strncpy(szFileName, g_szSourceFileName, sizeof(szFileName)-1);
	szFileName[sizeof(szFileName)-1] = '\0';
	if((fpSourceFile=fopen(szFileName,"r"))==NULL)
	{
		printf("Can not open source file %s",szFileName);
		return ERROR;
	}

	return OK;
}

void PrintSourceFile()//��ӡԴ�����ļ��е��������
{
char		cACharacter;

	fseek(fpSourceFile, 0L, SEEK_SET);//Դ�����ļ����ļ�ָ��ָ����ʼλ��

	while(fscanf(fpSourceFile,"%c",&cACharacter)!=EOF)//���ļ��ж�ȡһ���ַ�
		printf("%c",cACharacter);//��ӡ����ַ�
}


