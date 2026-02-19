#include "stdafx.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <thread>
#include <functional>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#ifdef _WIN32
  #include <direct.h>
#else
  #include <unistd.h>
#endif

#ifndef PL0_ENABLE_WEB_GUI
#define PL0_ENABLE_WEB_GUI 0
#endif

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #include <io.h>
  #pragma comment(lib, "ws2_32.lib")
  #define CLOSESOCK closesocket
  #define CLOSEFD   _close
  #define DUP      _dup
  #define DUP2     _dup2
  #define FILENO   _fileno
#else
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #include <fcntl.h>
  #define CLOSESOCK close
  #define CLOSEFD   close
  #define DUP      dup
  #define DUP2     dup2
  #define FILENO   fileno
#endif

// === 声明现有编译流水线的接口 ===
extern void SetSourceFileName(const char* szPath);
extern int Initialization();
extern void PrintSourceFile();
extern void LexicalAnalysis();
extern int ParsingAnalysis();
extern int GenerateAnalysis();
extern void Interpreter();
// 用于生成后校验跳转合法性
struct INSTRUCT_STRUCT { int eInstruct; int nLevel; double nOperand; };
extern INSTRUCT_STRUCT g_Instructs[];
extern int g_nInstructsIndex;

// 校验生成的指令跳转是否落在合法范围，避免未回填导致的死循环
static bool ValidateGeneratedInstructions()
{
    for(int i=0;i<g_nInstructsIndex;i++)
    {
        const auto& ins = g_Instructs[i];
        // 只检查跳转类指令
        if(ins.eInstruct==1 /*JPC*/ || ins.eInstruct==0 /*JMP*/)
        {
            int target = (int)ins.nOperand;
            if(target < 0 || target >= g_nInstructsIndex)
                return false;
        }
    }
    return true;
}

// === 简单工具：捕获 stdout 输出到字符串 ===
static std::string CaptureStdout(const std::function<void()>& fn)
{
    std::string tempPath = "http_output.log";
    // 备份当前 stdout
    int saved_fd = DUP(FILENO(stdout));
    FILE* f = std::fopen(tempPath.c_str(), "w+");
    if(!f) return "open temp log failed\n";
    DUP2(FILENO(f), FILENO(stdout));
    // 执行回调
    fn();
    std::fflush(stdout);
    // 恢复 stdout
    DUP2(saved_fd, FILENO(stdout));
    CLOSEFD(saved_fd);
    std::fclose(f);
    // 读取日志
    std::ifstream ifs(tempPath);
    std::stringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

// 仅保留解释器阶段的真实输出行（OUT/IN），过滤掉编译流程调试信息
static std::string FilterRuntimeOutput(const std::string& raw);

// === 跑一遍流水线，返回输出 ===
static std::string RunPipeline(const std::string& code)
{
    // 将代码写入固定文件，供现有编译流程读取（使用绝对路径，避免工作目录差异）
    char cwdBuf[1024];
#ifdef _WIN32
    if(!_getcwd(cwdBuf, sizeof(cwdBuf)))
        return "getcwd failed\n";
#else
    if(!getcwd(cwdBuf, sizeof(cwdBuf)))
        return "getcwd failed\n";
#endif
    std::string srcPath = std::string(cwdBuf) + "/HttpInput.pl0";
    std::ofstream ofs(srcPath, std::ios::binary);
    if(!ofs) return "open HttpInput.pl0 failed\n";
    ofs << code;
    ofs.close();

    SetSourceFileName(srcPath.c_str());

    auto runner = [](){
        if(Initialization()!=OK) { printf("INIT ERROR\n"); return; }
        PrintSourceFile();
        LexicalAnalysis();
        if(ParsingAnalysis()!=OK) { printf("PARSING ERROR\n"); return; }
        if(GenerateAnalysis()!=OK){ printf("GENERATE ERROR\n"); return; }
        if(!ValidateGeneratedInstructions()){ printf("GENERATE ERROR: INVALID JUMP\n"); return; }
        Interpreter();
    };
    // 捕获全量输出
    std::string raw = CaptureStdout(runner);
    // 只返回运行阶段的输出
    std::string filtered = FilterRuntimeOutput(raw);
    if(filtered.empty())
        return "（无输出）\n";
    return filtered;
}

// 仅保留运行相关的输出：解释器输出以及错误提示
static std::string FilterRuntimeOutput(const std::string& raw)
{
    std::stringstream in(raw);
    std::stringstream out;
    std::string line;
    while(std::getline(in, line))
    {
        // 保留包含运行期输入/输出提示或显式错误的行
        if(line.find("OUT_INSTRUCT") != std::string::npos ||
           line.find("IN_INSTRUCT")  != std::string::npos ||
           line.find("ERROR")        != std::string::npos)
        {
            out << line << "\n";
        }
    }
    return out.str();
}

// === 极简 HTTP 服务器 ===
static const char* kIndexHtml =
"<!doctype html>\n"
"<html>\n"
"<head><meta charset=\"utf-8\"><title>PL0 在线编译</title>\n"
"<style>\n"
"body { margin:20px; background:#fff; }\n"
".editor-container { display:flex; border:1px solid #ccc; font-family:monospace; font-size:14px; background:#fff; overflow:hidden; }\n"
".line-numbers { background:#f5f5f5; padding:8px 4px; text-align:right; user-select:none; color:#666; border-right:1px solid #ddd; overflow:hidden; }\n"
".line-number { line-height:1.5; }\n"
".line-number.error { background:#ffebee; color:#c62828; font-weight:bold; }\n"
"#src { flex:1; border:none; padding:8px; resize:none; outline:none; font-family:monospace; font-size:14px; line-height:1.5; background:#fff; overflow-y:auto; }\n"
"button { margin:12px 0; padding:8px 16px; background:#2196F3; color:#fff; border:none; border-radius:4px; cursor:pointer; font-size:14px; }\n"
"button:hover { background:#1976D2; }\n"
"button:active { background:#1565C0; }\n"
"#out { white-space:pre-wrap; border:1px solid #ccc; padding:8px; margin-top:12px; font-family:monospace; font-size:13px; background:#fff; min-height:50px; max-height:400px; overflow-y:auto; }\n"
"#out.error { background:#ffebee; border-color:#c62828; color:#c62828; }\n"
"</style>\n"
"</head>\n"
"<body>\n"
"<h3>输入 PL0 源码</h3>\n"
"<p style=\"color:#666\">（示例已移除，请直接在下方文本框输入/粘贴你的代码）</p>\n"
"<div class=\"editor-container\" style=\"height:400px;\">\n"
"  <div class=\"line-numbers\" id=\"lineNums\"></div>\n"
"  <textarea id=\"src\" placeholder=\"在此粘贴 PL0 代码...\" oninput=\"updateLineNumbers();\" onscroll=\"syncScroll();\"></textarea>\n"
"</div>\n"
"<br/>\n"
"<button onclick=\"run()\">编译并运行</button>\n"
"<pre id=\"out\"></pre>\n"
"<script>\n"
"function updateLineNumbers(){\n"
"  const src = document.getElementById('src');\n"
"  const nums = document.getElementById('lineNums');\n"
"  const lines = src.value.split('\\n');\n"
"  nums.innerHTML = lines.map((_,i)=>'<div class=\"line-number\" id=\"ln'+(i+1)+'\">'+(i+1)+'</div>').join('');\n"
"  src.style.height = 'auto';\n"
"  src.style.height = Math.max(400, src.scrollHeight) + 'px';\n"
"  nums.style.height = src.style.height;\n"
"}\n"
"function syncScroll(){\n"
"  const src = document.getElementById('src');\n"
"  const nums = document.getElementById('lineNums');\n"
"  nums.scrollTop = src.scrollTop;\n"
"}\n"
"function highlightErrorLine(lineNum){\n"
"  const nums = document.getElementById('lineNums');\n"
"  const all = nums.querySelectorAll('.line-number');\n"
"  all.forEach(el=>el.classList.remove('error'));\n"
"  if(lineNum>0 && lineNum<=all.length){\n"
"    all[lineNum-1].classList.add('error');\n"
"    const src = document.getElementById('src');\n"
"    const lines = src.value.split('\\n');\n"
"    if(lineNum<=lines.length){\n"
"      const lineHeight = parseFloat(getComputedStyle(src).lineHeight) || 21;\n"
"      src.scrollTop = (lineNum-1) * lineHeight - src.clientHeight/2;\n"
"    }\n"
"  }\n"
"}\n"
"function parseErrorLine(text){\n"
"  const m1 = text.match(/LEXICAL ERROR.*line (\\d+)/i);\n"
"  if(m1) return parseInt(m1[1]);\n"
"  const m2 = text.match(/SYNTAX ERROR.*line (\\d+)/i);\n"
"  if(m2) return parseInt(m2[1]);\n"
"  return 0;\n"
"}\n"
"async function run(){\n"
"  const out = document.getElementById('out');\n"
"  out.textContent = '编译中...';\n"
"  out.classList.remove('error');\n"
"  const nums = document.getElementById('lineNums');\n"
"  nums.querySelectorAll('.line-number').forEach(el=>el.classList.remove('error'));\n"
"  const resp = await fetch('/run?ts=' + Date.now(), {method:'POST',body:src.value, cache:'no-store'});\n"
"  const text = await resp.text();\n"
"  out.textContent = text;\n"
"  const errLine = parseErrorLine(text);\n"
"  if(errLine > 0){\n"
"    highlightErrorLine(errLine);\n"
"    out.classList.add('error');\n"
"  }\n"
"}\n"
"updateLineNumbers();\n"
"</script>\n"
"</body></html>\n";

static bool SendAll(int sock, const char* data, size_t len)
{
    size_t sent = 0;
    while(sent < len){
        int n = send(sock, data+sent, (int)(len - sent), 0);
        if(n<=0) return false;
        sent += n;
    }
    return true;
}

static void HandleClient(int client)
{
    char buf[8192];
    int n = recv(client, buf, sizeof(buf)-1, 0);
    if(n<=0){ CLOSESOCK(client); return; }
    buf[n]=0;

    std::string req(buf);
    bool isGet = req.rfind("GET /",0)==0;
    bool isPost = req.rfind("POST /run",0)==0;

    if(isGet){
        std::string body = kIndexHtml;
        std::string hdr = "HTTP/1.1 200 OK\r\n"
                          "Content-Type: text/html; charset=utf-8\r\n"
                          "Cache-Control: no-store, no-cache, must-revalidate\r\n"
                          "Pragma: no-cache\r\n"
                          "Content-Length: "+std::to_string(body.size())+"\r\n\r\n";
        SendAll(client, hdr.c_str(), hdr.size());
        SendAll(client, body.c_str(), body.size());
    }else if(isPost){
        // 解析 Content-Length（大小写不敏感），并确保按长度读取完整请求体
        size_t endhdr = req.find("\r\n\r\n");
        int contentLen = -1;
        if(endhdr != std::string::npos){
            std::string headers = req.substr(0, endhdr);
            std::stringstream hs(headers);
            std::string line;
            while(std::getline(hs, line)){
                // 移除可能的 \r
                if(!line.empty() && line.back()=='\r') line.pop_back();
                // 转小写比较
                std::string lower=line;
                for(char& c: lower) c = (char)tolower((unsigned char)c);
                if(lower.rfind("content-length:",0)==0){
                    std::string v = line.substr(line.find(":")+1);
                    contentLen = std::atoi(v.c_str());
                    break;
                }
            }
        }
        std::string body = (endhdr==std::string::npos)?std::string():req.substr(endhdr+4);
        if(contentLen<0) contentLen = (int)body.size(); // 回退：如果未找到长度，则使用已有数据长度
        while((int)body.size() < contentLen){
            int m = recv(client, buf, sizeof(buf), 0);
            if(m<=0) break;
            body.append(buf, buf+m);
        }
        std::string out = RunPipeline(body);
        std::string hdr = "HTTP/1.1 200 OK\r\n"
                          "Content-Type: text/plain; charset=utf-8\r\n"
                          "Cache-Control: no-store, no-cache, must-revalidate\r\n"
                          "Pragma: no-cache\r\n"
                          "Content-Length: "+std::to_string(out.size())+"\r\n\r\n";
        SendAll(client, hdr.c_str(), hdr.size());
        SendAll(client, out.c_str(), out.size());
    }else{
        const char* msg="HTTP/1.1 404 Not Found\r\nContent-Length:0\r\n\r\n";
        SendAll(client, msg, strlen(msg));
    }
    CLOSESOCK(client);
}

#if PL0_ENABLE_WEB_GUI
int main()
{
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd<0){ perror("socket"); return 1; }
    int opt=1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(server_fd,(sockaddr*)&addr,sizeof(addr))<0){ perror("bind"); return 1; }
    if(listen(server_fd, 8)<0){ perror("listen"); return 1; }
    printf("HTTP server started at http://localhost:8080\n");

    while(true){
        sockaddr_in caddr{};
        socklen_t clen = sizeof(caddr);
        int c = accept(server_fd,(sockaddr*)&caddr,&clen);
        if(c<0) continue;
        std::thread(HandleClient,c).detach();
    }
    CLOSESOCK(server_fd);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
#endif // PL0_ENABLE_WEB_GUI

