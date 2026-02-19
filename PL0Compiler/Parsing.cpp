#include "stdafx.h"

#define		MAX_LENGTH_OF_A_WORD				10		//一个单词的最多字符个数
#define		MAX_NUMBER_OF_WORDS					1000	//可识别的最多单词个数

enum	WORD_TYPE_ENUM{//单词类型枚举值（必须与Lexical.cpp中的定义顺序完全一致）
				INVALID_WORD,
				IDENTIFIER,
			INTEGER,        // 整数类型
			REAL,           // 实数类型
				CONST,
				VAR,
				PROCEDURE,
				BEGIN,
				END,
				IF,
				THEN,
			ELSE,           // else关键字
				WHILE,
				DO,
				WRITE,
				READ,
				CALL,
				LEFT_PARENTHESIS,
				RIGHT_PARENTHESIS,
				COMMA,
				SEMICOLON,
				PERIOD,
				PLUS,
				MINUS,
				MULTIPLY,
				DIVIDE,
				ODD,
				EQL,
				NEQ,
				LES,
				LEQ,
				GTR,
				GEQ,
				ASSIGN
};

struct	WORD_STRUCT{//一个单词的数据结构（必须与Lexical.cpp中的定义完全一致）
				char												szName[MAX_LENGTH_OF_A_WORD];//单词名字的字符串
				enum WORD_TYPE_ENUM		eType;//单词类型枚举值
				double												nNumberValue;//数单词的值（必须与Lexical.cpp一致，使用double）
				int												nLineNo;//在源代码文件中单词所在的行数
};

extern	WORD_STRUCT			g_Words[MAX_NUMBER_OF_WORDS];
extern	int									g_nWordsIndex;
extern	int									g_nInstructsIndex;

int BlockParsing(int nIndentNum);
int OneConstantParsing(int nIndentNum);
int OneVariableParsing(int nIndentNum);
int StatementParsing(int nIndentNum);
int ConditionParsing(int nIndentNum);
int ExpressionParsing(int nIndentNum);
int TermParsing(int nIndentNum);
int FactorParsing(int nIndentNum);
void PrintInParsing(int nWordsIndex,int nIndentNum,char *pString);
void ReportSyntaxError(const char* msg);

int ParsingAnalysis()//语法分析
{
	printf("\n----------------Parsing Begin!--------------\n");
	printf("No  Line--(Word)-----\n");

	g_nWordsIndex=0;//g_nWordsIndex是已识别单词的序号

	//程序由分程序Block开始
	PrintInParsing(g_nWordsIndex,0,"BlockParsing");//打印"BlockParsing"
	int nResult = BlockParsing(1);//"分程序"Block语法分析

	// 如果分程序本身分析失败，直接返回错误，不再继续
	if(nResult != OK)
	{
		printf("----------------Parsing Failed!--------------\n");
		return ERROR;
	}

	if(g_Words[g_nWordsIndex].eType==PERIOD)
	{//分程序Block后面以点'.'结束
		PrintInParsing(g_nWordsIndex,0,"PERIOD");//打印"PERIOD"
		printf("----------------Parsing Successfull Finished!--------------\n");
		return OK;
	}
	else
	{
		printf("----------------Parsing Failed! (missing PERIOD)--------------\n");
		return ERROR;
	}
}
//"分程序"Block语法分析
int BlockParsing(int nIndentNum)//nIndentNum是打印时要缩进的空格数
{
	// 跳过INVALID_WORD（空白字符等）
	while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
		g_nWordsIndex++;
	
	if(g_Words[g_nWordsIndex].eType==CONST)//如果单词是"const"
	{
		PrintInParsing(g_nWordsIndex,nIndentNum,"CONST");//打印调试信息

		g_nWordsIndex++;//取下一个单词
		PrintInParsing(g_nWordsIndex,nIndentNum,"OneConstantParsing");//打印调试信息
		int nResult = OneConstantParsing(nIndentNum+1);//一个"常量声明"语法分析
		if(nResult != OK)
			return ERROR;

		while(g_Words[g_nWordsIndex].eType==COMMA)//如果后面有逗号','
		{
			PrintInParsing(g_nWordsIndex,nIndentNum,"COMMA");//打印调试信息

			g_nWordsIndex++;//取下一个单词
			PrintInParsing(g_nWordsIndex,nIndentNum,"OneConstantParsing");//打印调试信息
			nResult = OneConstantParsing(nIndentNum+1);//必须再处理一个"常量声明"
			if(nResult != OK)
				return ERROR;
		}

		if(g_Words[g_nWordsIndex].eType==SEMICOLON)//常量声明最后以分号';'结束
		{
			PrintInParsing(g_nWordsIndex,nIndentNum,";");//打印调试信息
			g_nWordsIndex++;//取下一个单词
		}
		else
			return ERROR;//错误,漏掉了逗号或者分号
	}
	
	// 跳过INVALID_WORD（空白字符等）
	while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
		g_nWordsIndex++;
	
	if(g_Words[g_nWordsIndex].eType==VAR)//如果单词是"var"
	{
		PrintInParsing(g_nWordsIndex,nIndentNum,"VAR");//打印调试信息

		g_nWordsIndex++;//取下一个单词
		PrintInParsing(g_nWordsIndex,nIndentNum,"OneVariableParsing");//打印调试信息
		int nResult = OneVariableParsing(nIndentNum+1);//一个"变量声明"语法分析
		if(nResult != OK)
			return ERROR;

		while(g_Words[g_nWordsIndex].eType==COMMA)//如果后面有逗号','
		{
			PrintInParsing(g_nWordsIndex,nIndentNum,"COMMA");//打印调试信息

			g_nWordsIndex++;//取下一个单词
			PrintInParsing(g_nWordsIndex,nIndentNum,"OneVariableParsing");//打印调试信息
			nResult = OneVariableParsing(nIndentNum+1);//必须再处理一个"变量声明"
			if(nResult != OK)
				return ERROR;
		}

		if(g_Words[g_nWordsIndex].eType==SEMICOLON)//变量声明最后以分号';'结束
		{
			PrintInParsing(g_nWordsIndex,nIndentNum,";");//打印调试信息
			g_nWordsIndex++;//取下一个单词
		}
		else
			return ERROR;
	}
	
	// 跳过INVALID_WORD（空白字符等）
	while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
		g_nWordsIndex++;
	
	while (g_Words[g_nWordsIndex].eType==PROCEDURE)//如果单词是"procedure",则进入"函数定义"语法分析
	{
		PrintInParsing(g_nWordsIndex,nIndentNum,"PROCEDURE");//打印调试信息

		g_nWordsIndex++;//取下一个单词
		if(g_Words[g_nWordsIndex].eType==IDENTIFIER)//"procedure"后应为标识符
		{
			PrintInParsing(g_nWordsIndex,nIndentNum,"IDENTIFIER");//打印调试信息
			g_nWordsIndex++;//取下一个单词
		}
		else
			return ERROR;

		if(g_Words[g_nWordsIndex].eType==SEMICOLON)//标识符后应是分号';'
		{
			PrintInParsing(g_nWordsIndex,nIndentNum,";");//打印调试信息
			g_nWordsIndex++;//取下一个单词
		}
		else
			return ERROR;//漏掉了分号

		PrintInParsing(g_nWordsIndex,nIndentNum,"BlockParsing");//打印调试信息
		BlockParsing(nIndentNum+1);//在"函数定义"中,分号后面是"分程序"Block

		if(g_Words[g_nWordsIndex].eType==SEMICOLON)//"分程序"Block后面应是分号';'
		{
			PrintInParsing(g_nWordsIndex,nIndentNum,";");//打印调试信息
			g_nWordsIndex++;//取下一个单词
		}
		else
			return ERROR;//漏掉了分号
	}
	
	// 跳过INVALID_WORD（空白字符等）
	while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
		g_nWordsIndex++;
	
	PrintInParsing(g_nWordsIndex,nIndentNum,"StatementParsing");//打印调试信息
	// "分程序"Block中必须有"语句"，且语句分析失败时应立即返回错误
	{
		int nResult = StatementParsing(nIndentNum+1);
		if(nResult != OK)
			return ERROR;
	}

	return OK;
}

//一个"常量声明"语法分析
int OneConstantParsing(int nIndentNum)//nIndentNum是打印时要缩进的空格数
{
	if(g_Words[g_nWordsIndex].eType==IDENTIFIER)//一个常量声明以标识符开头
	{
		PrintInParsing(g_nWordsIndex,nIndentNum,"IDENTIFIER");//打印调试信息

		g_nWordsIndex++;//取下一个单词
		if(g_Words[g_nWordsIndex].eType==EQL)//标识符后面应是'='
		{
			PrintInParsing(g_nWordsIndex,nIndentNum,"EQL");//打印调试信息

			g_nWordsIndex++;//取下一个单词
			// '='后面的单词应是数（支持INTEGER或REAL类型）
			if(g_Words[g_nWordsIndex].eType==INTEGER)
			{
				PrintInParsing(g_nWordsIndex,nIndentNum,"INTEGER");//打印调试信息

				g_nWordsIndex++;//取下一个单词
				return OK;
			}
			else if(g_Words[g_nWordsIndex].eType==REAL)
			{
				PrintInParsing(g_nWordsIndex,nIndentNum,"REAL");//打印调试信息

				g_nWordsIndex++;//取下一个单词
				return OK;
			}
            else
			{
				// 打印错误信息以便调试
				PrintInParsing(g_nWordsIndex,nIndentNum,"ERROR: Expected INTEGER or REAL");
				return ERROR;
			}
		}
		else
		{
			// 打印错误信息以便调试
			PrintInParsing(g_nWordsIndex,nIndentNum,"ERROR: Expected EQL");
			return ERROR;
		}
	}
	else
	{
		// 打印错误信息以便调试
		PrintInParsing(g_nWordsIndex,nIndentNum,"ERROR: Expected IDENTIFIER");
		return ERROR;
	}

	return ERROR;
}
//一个"变量声明"语法分析
int OneVariableParsing(int nIndentNum)//nIndentNum是打印时要缩进的空格数
{
	if(g_Words[g_nWordsIndex].eType==IDENTIFIER)//一个变量声明就是标识符
	{
		PrintInParsing(g_nWordsIndex,nIndentNum,"IDENTIFIER");//打印调试信息
		g_nWordsIndex++;//取下一个单词

		return OK;
	}
	else
		return ERROR;

	return ERROR;
}
//一条"语句"语法分析
int StatementParsing(int nIndentNum)//nIndentNum是打印时要缩进的空格数
{
	int		nResult;

	// 跳过INVALID_WORD（空白字符等）
	while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
		g_nWordsIndex++;

	switch(g_Words[g_nWordsIndex].eType)//当前单词类型是
	{
	case IDENTIFIER://如果第一个单词是标识符,应按照"赋值语句"处理
				PrintInParsing(g_nWordsIndex,nIndentNum,"IDENTIFIER");//打印调试信息

				g_nWordsIndex++;//取下一个单词
				if(g_Words[g_nWordsIndex].eType==ASSIGN)//标识符后面应是赋值号单词":="
				{
					PrintInParsing(g_nWordsIndex,nIndentNum,"ASSIGN");//打印调试信息

					g_nWordsIndex++;//取下一个单词
					PrintInParsing(g_nWordsIndex,nIndentNum,"ExpressionParsing");//打印调试信息
					nResult=ExpressionParsing(nIndentNum+1);//赋值号单词":="后面应是表达式

					if(nResult != OK)
						return ERROR;

					// 赋值语句必须以分号;结束
					if(g_Words[g_nWordsIndex].eType==SEMICOLON)
					{
						PrintInParsing(g_nWordsIndex,nIndentNum,";");//打印调试信息
						g_nWordsIndex++;//取下一个单词
						return OK;
					}
					else
						return ERROR;//缺少分号
				}
				else
					return ERROR;//没有检测到':='符号

				break;

	case IF://如果第一个单词是"if",按"条件语句"处理
				PrintInParsing(g_nWordsIndex,nIndentNum,"IF");//打印调试信息

				g_nWordsIndex++;//取下一个单词
				// "条件"前必须要有左括号'('
				if(g_Words[g_nWordsIndex].eType==LEFT_PARENTHESIS)
				{
					PrintInParsing(g_nWordsIndex,nIndentNum,"LEFT_PARENTHESIS");//打印调试信息
					g_nWordsIndex++;//取下一个单词
				}
				else
					return ERROR;//缺少左括号

				PrintInParsing(g_nWordsIndex,nIndentNum,"ConditionParsing");//打印调试信息
				nResult=ConditionParsing(nIndentNum+1);//"("后是"条件"Condition
				if(nResult != OK)
					return ERROR;

				// "条件"后必须要有右括号')'
				if(g_Words[g_nWordsIndex].eType==RIGHT_PARENTHESIS)
				{
					PrintInParsing(g_nWordsIndex,nIndentNum,"RIGHT_PARENTHESIS");//打印调试信息
					g_nWordsIndex++;//取下一个单词
				}
				else
					return ERROR;//缺少右括号

				// 删除"then"，直接是语句
				PrintInParsing(g_nWordsIndex,nIndentNum,"StatementParsing");//打印调试信息
				nResult=StatementParsing(nIndentNum+1);//")"后应是"语句"
				if(nResult != OK)
					return ERROR;

				// 跳过INVALID_WORD（空白字符等）
				while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
					g_nWordsIndex++;

				// 如果当前是分号，说明语句已经结束，需要跳过分号检查else
				// 注意：某些语句（如write）不消费分号，分号由上层处理
				if(g_Words[g_nWordsIndex].eType==SEMICOLON)
				{
					g_nWordsIndex++;//跳过分号
					// 跳过INVALID_WORD（空白字符等）
					while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
						g_nWordsIndex++;
				}

				// 调试：打印当前单词信息，检查else
				printf("DEBUG IF: g_nWordsIndex=%d, eType=%d, szName='%s', ELSE=%d\n", 
					g_nWordsIndex, g_Words[g_nWordsIndex].eType, g_Words[g_nWordsIndex].szName, ELSE);

				// 支持可选的else分支
				if(g_Words[g_nWordsIndex].eType==ELSE)
				{
					PrintInParsing(g_nWordsIndex,nIndentNum,"ELSE");//打印调试信息
					g_nWordsIndex++;//取下一个单词
					PrintInParsing(g_nWordsIndex,nIndentNum,"StatementParsing");//打印调试信息
					nResult=StatementParsing(nIndentNum+1);//"else"后应是"语句"
					return nResult;
				}

				return OK;

				break;

	case WHILE://如果第一个单词是"while",准备按照"循环语句"处理
				PrintInParsing(g_nWordsIndex,nIndentNum,"WHILE");//打印调试信息

				g_nWordsIndex++;//取下一个单词
				
				// 跳过INVALID_WORD（空白字符等）
				while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
					g_nWordsIndex++;

				PrintInParsing(g_nWordsIndex,nIndentNum,"ConditionParsing");//打印调试信息
				nResult=ConditionParsing(nIndentNum+1);//"while"后面应是"条件"
				if(nResult != OK)
					return ERROR;

				// 跳过INVALID_WORD（空白字符等）
				while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
					g_nWordsIndex++;

				if(g_Words[g_nWordsIndex].eType==DO)//"条件"后面应是单词"do"
				{
					PrintInParsing(g_nWordsIndex,nIndentNum,"DO");//打印调试信息

					g_nWordsIndex++;//取下一个单词
					
					// 跳过INVALID_WORD（空白字符等）
					while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
						g_nWordsIndex++;

					PrintInParsing(g_nWordsIndex,nIndentNum,"StatementParsing");//打印调试信息
					nResult=StatementParsing(nIndentNum+1);//"do"后面应是"一条语句"
					if(nResult != OK)
						return ERROR;

					// 跳过INVALID_WORD（空白字符等）
					while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
						g_nWordsIndex++;

					return OK;
				}
				else
					return ERROR;//缺少do

				break;

	case DO://如果第一个单词是"do",按照"do-while循环语句"处理
				PrintInParsing(g_nWordsIndex,nIndentNum,"DO");//打印调试信息

				g_nWordsIndex++;//取下一个单词

				// 跳过INVALID_WORD（空白字符等）
				while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
					g_nWordsIndex++;

				PrintInParsing(g_nWordsIndex,nIndentNum,"StatementParsing");//打印调试信息
				nResult=StatementParsing(nIndentNum+1);//"do"后面应是"一条语句"
				if(nResult != OK)
					return ERROR;

				// 跳过INVALID_WORD（空白字符等）
				while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
					g_nWordsIndex++;

				if(g_Words[g_nWordsIndex].eType==WHILE)//"语句"后面应是单词"while"
				{
					PrintInParsing(g_nWordsIndex,nIndentNum,"WHILE");//打印调试信息

					g_nWordsIndex++;//取下一个单词

					// 跳过INVALID_WORD（空白字符等）
					while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
						g_nWordsIndex++;

					PrintInParsing(g_nWordsIndex,nIndentNum,"ConditionParsing");//打印调试信息
					nResult=ConditionParsing(nIndentNum+1);//"while"后面应是"条件"
					if(nResult != OK)
						return ERROR;

					// 跳过INVALID_WORD（空白字符等）
					while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
						g_nWordsIndex++;

					if(g_Words[g_nWordsIndex].eType==SEMICOLON)//do-while语句整体以分号结束
					{
						PrintInParsing(g_nWordsIndex,nIndentNum,";");//打印调试信息
						g_nWordsIndex++;//取下一个单词
						return OK;
					}
					else
						return ERROR;//缺少分号
				}
				else
					return ERROR;//缺少while

				break;

	case BEGIN://如果第一个单词是"begin",准备按照"复合语句"处理
				PrintInParsing(g_nWordsIndex,nIndentNum,"BEGIN");//打印调试信息

				g_nWordsIndex++;//取下一个单词
				
				// 跳过INVALID_WORD（空白字符等）
				while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
					g_nWordsIndex++;

				PrintInParsing(g_nWordsIndex,nIndentNum,"StatementParsing");//打印调试信息
				{
					int nResult = StatementParsing(nIndentNum+1);//"begin"后面应是一条语句
					if(nResult != OK)
						return ERROR;
				}

				// 跳过INVALID_WORD（空白字符等），然后检查是否是分号或下一条语句
				while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
					g_nWordsIndex++;

				// 调试：打印当前单词信息
				printf("DEBUG BEGIN: g_nWordsIndex=%d, eType=%d, szName='%s', ELSE=%d, END=%d\n", 
					g_nWordsIndex, g_Words[g_nWordsIndex].eType, g_Words[g_nWordsIndex].szName, ELSE, END);

				// 持续解析后续语句，必须以END结束；若未遇到END则报错
				while(true)
				{
					// 跳过INVALID_WORD（空白字符等）
					while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
						g_nWordsIndex++;

					// 若当前是END则结束复合语句
					if(g_Words[g_nWordsIndex].eType==END)
					{
						PrintInParsing(g_nWordsIndex,nIndentNum,"END");//打印调试信息
						g_nWordsIndex++;//取下一个单词
						return OK;
					}

					// 如果当前是分号，先消费分号再看下一条语句
					if(g_Words[g_nWordsIndex].eType==SEMICOLON)
					{
						PrintInParsing(g_nWordsIndex,nIndentNum,";");//打印调试信息
						g_nWordsIndex++;
						continue;
					}

					// 若不是语句起始符且也不是END，视为缺少END
					if(!(g_Words[g_nWordsIndex].eType==IDENTIFIER || 
				        g_Words[g_nWordsIndex].eType==IF ||
				        g_Words[g_nWordsIndex].eType==WHILE ||
				        g_Words[g_nWordsIndex].eType==DO ||
				        g_Words[g_nWordsIndex].eType==BEGIN ||
				        g_Words[g_nWordsIndex].eType==READ ||
				        g_Words[g_nWordsIndex].eType==WRITE ||
					     g_Words[g_nWordsIndex].eType==CALL))
				{
						// 精确报错：当前token行号、内容，以及提示缺少END
						ReportSyntaxError("missing 'end' to match previous 'begin'");
						return ERROR;// 非法的语句起始，缺少END
					}

					// 解析下一条语句
					PrintInParsing(g_nWordsIndex,nIndentNum,"StatementParsing");//打印调试信息
					if(StatementParsing(nIndentNum+1)!=OK)
						return ERROR;
				}

				break;

	case READ://如果第一个单词是"read",准备按照"读语句"处理
				PrintInParsing(g_nWordsIndex,nIndentNum,"READ");//打印调试信息

				g_nWordsIndex++;//取下一个单词
				if(g_Words[g_nWordsIndex].eType==LEFT_PARENTHESIS)//"read"后面应是左括号'('
				{
					PrintInParsing(g_nWordsIndex,nIndentNum,"LEFT_PARENTHESIS");//打印调试信息
					do{//一条"读语句"应至少有一个变量标识符
						g_nWordsIndex++;//取下一个单词

						if(g_Words[g_nWordsIndex].eType==IDENTIFIER)//后面应是一个标识符
						{
							PrintInParsing(g_nWordsIndex,nIndentNum,"IDENTIFIER");//打印调试信息
							g_nWordsIndex++;//取下一个单词
						}
						else
							return ERROR;//格式错误,

						if(g_Words[g_nWordsIndex].eType==COMMA)//打印调试信息
							PrintInParsing(g_nWordsIndex,nIndentNum,"COMMA");
					}while(g_Words[g_nWordsIndex].eType==COMMA);//如果标识符后面还有一个逗号','

					if(g_Words[g_nWordsIndex].eType==RIGHT_PARENTHESIS)//最后应是右括号')'
					{
						PrintInParsing(g_nWordsIndex,nIndentNum,"RIGHT_PARENTHESIS");//打印调试信息
						g_nWordsIndex++;//取下一个单词
						return OK;
					}
					else
						return ERROR;//格式错误,应是右括号
				}
				else
					return ERROR;//格式错误,应是左括号

				break;

	case WRITE://如果第一个单词是"write",准备按照"写语句"处理
				PrintInParsing(g_nWordsIndex,nIndentNum,"WRITE");//打印调试信息

				g_nWordsIndex++;//取下一个单词
				if(g_Words[g_nWordsIndex].eType==LEFT_PARENTHESIS)//"write"后面应是左括号'('
				{
					PrintInParsing(g_nWordsIndex,nIndentNum,"(");//打印调试信息

					do{//一条"写语句"应至少有一个或多个变量标识符
						g_nWordsIndex++;//取下一个单词
						if(g_Words[g_nWordsIndex].eType==IDENTIFIER)//后面应是一个标识符
						{
							PrintInParsing(g_nWordsIndex,nIndentNum,"IDENTIFIER");//打印调试信息
							g_nWordsIndex++;//取下一个单词
						}
						else
							return ERROR;//格式错误,

						if(g_Words[g_nWordsIndex].eType==COMMA)//打印调试信息
							PrintInParsing(g_nWordsIndex,nIndentNum,"COMMA");
					}while(g_Words[g_nWordsIndex].eType==COMMA);//如果标识符后面有一个逗号','

					if(g_Words[g_nWordsIndex].eType==RIGHT_PARENTHESIS)//最后应是右括号')'
					{
						PrintInParsing(g_nWordsIndex,nIndentNum,")");//打印调试信息

						g_nWordsIndex++;//取下一个单词
						return OK;
					}
					else
						return ERROR;//格式错误,应是右括号
				}
				else
					return ERROR;//格式错误,应是左括号

				break;

	case CALL://如果第一个单词是"call",准备按照"调用函数语句"处理
				PrintInParsing(g_nWordsIndex,nIndentNum,"CALL");//打印调试信息

				g_nWordsIndex++;//取下一个单词
				if(g_Words[g_nWordsIndex].eType==IDENTIFIER)//"call"后面应是一个标识符,过程名
				{
					PrintInParsing(g_nWordsIndex,nIndentNum,"IDENTIFIER");//打印调试信息

					g_nWordsIndex++;//取下一个单词
					return OK;
				}
				else
					return ERROR;

				break;

	default:	return ERROR;
	}

	return ERROR;
}
//"条件"语法分析
int ConditionParsing(int nIndentNum)//nIndentNum是打印时要缩进的空格数
{
	// 跳过INVALID_WORD（空白字符等）
	while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
		g_nWordsIndex++;

	if(g_Words[g_nWordsIndex].eType==ODD)//如果有"odd"单词
	{
		PrintInParsing(g_nWordsIndex,nIndentNum,"ODD");//打印调试信息

		g_nWordsIndex++;//取下一个单词
		
		// 跳过INVALID_WORD（空白字符等）
		while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
			g_nWordsIndex++;

		PrintInParsing(g_nWordsIndex,nIndentNum,"ExpressionParsing");//打印调试信息
		int nResult = ExpressionParsing(nIndentNum+1);//"odd"单词后面应有一个"表达式"
		if(nResult != OK)
			return ERROR;
	}
	else
	{
		PrintInParsing(g_nWordsIndex,nIndentNum,"ExpressionParsing");//打印调试信息
		int nResult = ExpressionParsing(nIndentNum+1);//条件可以是一个"表达式"
		if(nResult != OK)
			return ERROR;

		// 跳过INVALID_WORD（空白字符等）
		while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
			g_nWordsIndex++;

		if(		g_Words[g_nWordsIndex].eType==EQL		//如果后面有逻辑运算符
			||	g_Words[g_nWordsIndex].eType==NEQ
			||	g_Words[g_nWordsIndex].eType==LES 
			||	g_Words[g_nWordsIndex].eType==LEQ
			||	g_Words[g_nWordsIndex].eType==GTR
			||	g_Words[g_nWordsIndex].eType==GEQ)
		{
			PrintInParsing(g_nWordsIndex,nIndentNum,"LOGICAL_OP");//打印调试信息

			g_nWordsIndex++;//取下一个单词
			
			// 跳过INVALID_WORD（空白字符等）
			while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
				g_nWordsIndex++;

			PrintInParsing(g_nWordsIndex,nIndentNum,"ExpressionParsing");//打印调试信息
			nResult = ExpressionParsing(nIndentNum+1);//则继续处理后面第二个"表达式"
			if(nResult != OK)
				return ERROR;
		}
		else
			return ERROR;//缺少关系运算符
	}
	return OK;
}
//"表达式"语法分析
int ExpressionParsing(int nIndentNum)//nIndentNum是打印时要缩进的空格数
{
	// 跳过INVALID_WORD（空白字符等）
	while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
		g_nWordsIndex++;

	if(g_Words[g_nWordsIndex].eType==PLUS || g_Words[g_nWordsIndex].eType==MINUS)
	{//如果开头有正负号,则此时表达式应被看作一个正的或负的项
		PrintInParsing(g_nWordsIndex,nIndentNum,"PLUS/MINUS");//打印调试信息
		g_nWordsIndex++;//取下一个单词
		
		// 跳过INVALID_WORD（空白字符等）
		while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
			g_nWordsIndex++;
	}

	PrintInParsing(g_nWordsIndex,nIndentNum,"TermParsing");//打印调试信息
	TermParsing(nIndentNum+1);//表达式可以是一个"项"

	// 跳过INVALID_WORD（空白字符等）
	while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
		g_nWordsIndex++;

	while(g_Words[g_nWordsIndex].eType==PLUS || g_Words[g_nWordsIndex].eType==MINUS)
	{//如果"项"后面还有'+'或'-'
		PrintInParsing(g_nWordsIndex,nIndentNum,"PLUS/MINUS");//打印调试信息

		g_nWordsIndex++;//取下一个单词
		
		// 跳过INVALID_WORD（空白字符等）
		while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
			g_nWordsIndex++;

		PrintInParsing(g_nWordsIndex,nIndentNum,"TermParsing");//打印调试信息
		TermParsing(nIndentNum+1);//则后面还有"项"
		
		// 跳过INVALID_WORD（空白字符等）
		while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
			g_nWordsIndex++;
	}
	return OK;
}
//"项"语法分析
int TermParsing(int nIndentNum)//nIndentNum是打印时要缩进的空格数
{
	// 跳过INVALID_WORD（空白字符等）
	while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
		g_nWordsIndex++;

	PrintInParsing(g_nWordsIndex,nIndentNum,"FactorParsing");//打印调试信息
	FactorParsing(nIndentNum+1);//"项"可以是一个"因子"

	// 跳过INVALID_WORD（空白字符等）
	while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
		g_nWordsIndex++;

	while(g_Words[g_nWordsIndex].eType==MULTIPLY || g_Words[g_nWordsIndex].eType==DIVIDE)
	{//如果"因子"后面还有单词'*'或'/'
		PrintInParsing(g_nWordsIndex,nIndentNum,"MULTI/DIVISION");//打印调试信息

		g_nWordsIndex++;//取下一个单词
		
		// 跳过INVALID_WORD（空白字符等）
		while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
			g_nWordsIndex++;

		PrintInParsing(g_nWordsIndex,nIndentNum,"FactorParsing");//打印调试信息
		FactorParsing(nIndentNum+1);//则后面还有"因子"
		
		// 跳过INVALID_WORD（空白字符等）
		while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
			g_nWordsIndex++;
	}
	return OK;
}
//"因子"语法分析
int FactorParsing(int nIndentNum)
{
	// 跳过INVALID_WORD（空白字符等）
	while(g_Words[g_nWordsIndex].eType==INVALID_WORD)
		g_nWordsIndex++;

	switch(g_Words[g_nWordsIndex].eType)
	{
	case IDENTIFIER://因子可以是一个常量或变量
					PrintInParsing(g_nWordsIndex,nIndentNum,"IDENTIFIER");//打印调试信息

					g_nWordsIndex++;//取下一个单词
					return OK;

	case INTEGER:	//因子可以是一个整数
					PrintInParsing(g_nWordsIndex,nIndentNum,"INTEGER");//打印调试信息

					g_nWordsIndex++;//取下一个单词
					return OK;
	case REAL:		//因子可以是一个实数
					PrintInParsing(g_nWordsIndex,nIndentNum,"REAL");//打印调试信息

					g_nWordsIndex++;//取下一个单词
					return OK;

	case LEFT_PARENTHESIS:	//如果看到左括号'('
					PrintInParsing(g_nWordsIndex,nIndentNum,"(");//打印调试信息

					g_nWordsIndex++;//取下一个单词
					PrintInParsing(g_nWordsIndex,nIndentNum,"ExpressionParsing");//打印调试信息
					ExpressionParsing(nIndentNum+1);//左括号'('后面应是"表达式"

					if(g_Words[g_nWordsIndex].eType==RIGHT_PARENTHESIS)//"表达式"后面应是右括号')'
					{
						PrintInParsing(g_nWordsIndex,nIndentNum,")");//打印调试信息

						g_nWordsIndex++;//取下一个单词
						return OK;
					}
					else
						return ERROR;//缺少右括号
					break;

	default:		return ERROR;
	}
	return OK;
}
void PrintInParsing(int nWordsIndex,int nIndentNum,char *pString)//打印调试信息
{
	int		i;
	char	strText[100];

	printf("%-4d%-4d%-12s",nWordsIndex+1,g_Words[nWordsIndex].nLineNo,g_Words[nWordsIndex].szName);
	printf("%-1s","|");
	for (i=0;i<nIndentNum;i++)
		printf("  ");
	printf("%-1s"," ");

	sprintf(strText,"(%d)%s",nIndentNum,pString);
	printf("%-15s\n",strText);
}

// 统一的语法错误报告函数：打印当前单词序号、行号和内容
void ReportSyntaxError(const char* msg)
{
	if(g_nWordsIndex < 0)
		return;
	int idx = g_nWordsIndex;
	printf("\nSYNTAX ERROR at token %d (line %d, '%s'): %s\n",
		   idx+1,
		   g_Words[idx].nLineNo,
		   g_Words[idx].szName,
		   msg);
}
